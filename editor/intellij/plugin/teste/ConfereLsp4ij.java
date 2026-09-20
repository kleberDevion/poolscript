import com.google.gson.Gson;
import com.redhat.devtools.lsp4ij.LanguageServerFactory;
import com.redhat.devtools.lsp4ij.server.ProcessStreamConnectionProvider;
import org.eclipse.lsp4j.ClientCapabilities;
import org.eclipse.lsp4j.CompletionItem;
import org.eclipse.lsp4j.CompletionList;
import org.eclipse.lsp4j.CompletionParams;
import org.eclipse.lsp4j.DidOpenTextDocumentParams;
import org.eclipse.lsp4j.InitializeParams;
import org.eclipse.lsp4j.InitializedParams;
import org.eclipse.lsp4j.MessageActionItem;
import org.eclipse.lsp4j.MessageParams;
import org.eclipse.lsp4j.Position;
import org.eclipse.lsp4j.PublishDiagnosticsParams;
import org.eclipse.lsp4j.ShowMessageRequestParams;
import org.eclipse.lsp4j.TextDocumentIdentifier;
import org.eclipse.lsp4j.TextDocumentItem;
import org.eclipse.lsp4j.jsonrpc.Launcher;
import org.eclipse.lsp4j.jsonrpc.messages.Either;
import org.eclipse.lsp4j.launch.LSPLauncher;
import org.eclipse.lsp4j.services.LanguageClient;
import org.eclipse.lsp4j.services.LanguageServer;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import javax.xml.parsers.DocumentBuilderFactory;
import java.io.File;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/**
 * Confere o plugin do IntelliJ contra os jars REAIS do IDEA e do LSP4IJ — não
 * contra os stubs. Stub que não casa com o binário só quebra em runtime, e
 * aqui é o runtime: as classes reais, o LSP4J real, o servidor instalado.
 *
 *     ConfereLsp4ij <jar do plugin> <poolscript-lsp resolvido ou ""> <pool resolvido ou "">
 *
 * Chamado por teste_intellij.sh, com o java do JBR do IDEA e o classpath do
 * plugin + lsp4ij/lib + IDEA/lib. Sai 1 se algo falhou.
 */
public class ConfereLsp4ij {
  static int falhas = 0;

  static void ok(String s) { System.out.println("  ok     " + s); }
  static void falha(String s, Object detalhe) {
    System.out.println("  FALHOU " + s);
    if (detalhe != null) System.out.println("         " + String.valueOf(detalhe));
    falhas++;
  }
  static void confere(String nome, boolean cond, Object detalhe) { if (cond) ok(nome); else falha(nome, detalhe); }

  static String lerDoJar(JarFile jar, String nome) throws Exception {
    JarEntry e = jar.getJarEntry(nome);
    if (e == null) return null;
    try (InputStream in = jar.getInputStream(e)) { return new String(in.readAllBytes(), StandardCharsets.UTF_8); }
  }

  public static void main(String[] args) throws Exception {
    String jarPath = args[0];
    String lsp = args.length > 1 ? args[1] : "";
    String pool = args.length > 2 ? args[2] : "";

    /* 1. o jar so tem o que e nosso: stub embarcado sombrearia a classe real */
    try (JarFile jar = new JarFile(jarPath)) {
      List<String> vazados = new ArrayList<>();
      for (Enumeration<JarEntry> en = jar.entries(); en.hasMoreElements();) {
        String n = en.nextElement().getName();
        if (n.startsWith("com/") || n.startsWith("org/")) vazados.add(n);
      }
      confere("jar sem stub vazado (nada em com/ ou org/)", vazados.isEmpty(), vazados);

      /* 2. plugin.xml declara a dependencia OPCIONAL do LSP4IJ com o config-file */
      String px = lerDoJar(jar, "META-INF/plugin.xml");
      confere("plugin.xml tem <depends optional config-file=\"poolscript-lsp4ij.xml\">com.redhat.devtools.lsp4ij",
              px != null && px.contains("config-file=\"poolscript-lsp4ij.xml\"") && px.contains("com.redhat.devtools.lsp4ij")
                && px.contains("optional=\"true\""), px);

      /* 3. o XML do EP: server@id == mapping@serverId, *.pr, languageId poolscript */
      String lx = lerDoJar(jar, "META-INF/poolscript-lsp4ij.xml");
      confere("poolscript-lsp4ij.xml esta no jar", lx != null, null);
      String factoryClass = null;
      if (lx != null) {
        Document d = DocumentBuilderFactory.newInstance().newDocumentBuilder()
            .parse(new java.io.ByteArrayInputStream(lx.getBytes(StandardCharsets.UTF_8)));
        Element server = (Element) d.getElementsByTagName("server").item(0);
        Element map = (Element) d.getElementsByTagName("fileNamePatternMapping").item(0);
        factoryClass = server != null ? server.getAttribute("factoryClass") : null;
        confere("server@id == fileNamePatternMapping@serverId",
                server != null && map != null && server.getAttribute("id").equals(map.getAttribute("serverId")),
                server == null ? "sem <server>" : map == null ? "sem <fileNamePatternMapping>" : server.getAttribute("id") + " x " + map.getAttribute("serverId"));
        confere("mapping cobre *.pr", map != null && (";" + map.getAttribute("patterns") + ";").contains(";*.pr;"),
                map != null ? map.getAttribute("patterns") : null);
        confere("mapping manda languageId=poolscript no didOpen", map != null && "poolscript".equals(map.getAttribute("languageId")),
                map != null ? map.getAttribute("languageId") : null);
      }

      /* 4. a factory implementa a interface REAL do LSP4IJ */
      Class<?> f = factoryClass != null ? Class.forName(factoryClass) : null;
      confere("factoryClass carrega e implementa LanguageServerFactory (real)",
              f != null && LanguageServerFactory.class.isAssignableFrom(f), factoryClass);
      if (f == null) { System.exit(1); return; }

      /* 5. o provedor: ProcessStreamConnectionProvider real, comando absoluto + --stdio, pasta existente */
      LanguageServerFactory fab = (LanguageServerFactory) f.getDeclaredConstructor().newInstance();
      Object prov = fab.createConnectionProvider(null);
      confere("createConnectionProvider devolve um ProcessStreamConnectionProvider (real)",
              prov instanceof ProcessStreamConnectionProvider, prov == null ? "null" : prov.getClass().getName());
      if (!(prov instanceof ProcessStreamConnectionProvider)) { System.exit(1); return; }
      ProcessStreamConnectionProvider p = (ProcessStreamConnectionProvider) prov;
      List<String> cmd = p.getCommands();
      confere("comando = poolscript-lsp resolvido + --stdio",
              cmd != null && cmd.size() == 2 && cmd.get(1).equals("--stdio")
                && (lsp.isEmpty() ? cmd.get(0).equals("poolscript-lsp") : cmd.get(0).equals(lsp)), cmd);
      confere("pasta de trabalho existe", p.getWorkingDirectory() != null && new File(p.getWorkingDirectory()).isDirectory(),
              p.getWorkingDirectory());

      /* 6. initializationOptions serializado pelo Gson REAL do IDEA */
      Object opts = p.getInitializationOptions(null);
      String json = new Gson().toJson(opts);
      String esperado = pool.isEmpty() ? "{}" : "{\"pool\":\"" + pool + "\"}";
      confere("initializationOptions vira " + esperado, json.equals(esperado), json);

      /* 7. a busca do executavel: fallback fora do PATH, e null pro que nao existe */
      Method acha = f.getDeclaredMethod("acha", String.class, String.class);
      acha.setAccessible(true);
      Object porFallback = acha.invoke(null, "poolscript-lsp", "/nao/existe");
      confere("acha() cai nas pastas de fallback quando o PATH nao tem o poolscript-lsp",
              lsp.isEmpty() ? porFallback == null : lsp.equals(porFallback), porFallback);
      confere("acha() devolve null pro que nao existe", acha.invoke(null, "xyz-nao-existe-123", System.getenv("PATH")) == null, null);

      /* 8. ponta a ponta com o LSP4J de verdade, como o LSP4IJ faz */
      if (lsp.isEmpty()) {
        System.out.println("  PULOU  ponta a ponta — falta `poolscript-lsp` (rode `sudo make install`)");
      } else {
        pontaAPonta(p, opts);
      }
    }

    System.out.println();
    if (falhas > 0) { System.out.println("intellij: " + falhas + " checagem(ns) FALHARAM"); System.exit(1); }
    System.out.println("intellij: o plugin liga com o IDEA e o LSP4IJ reais, e o servidor responde ao LSP4J");
  }

  static void pontaAPonta(ProcessStreamConnectionProvider p, Object opts) throws Exception {
    Path dir = Files.createTempDirectory("ps_ij_");
    Path arq = dir.resolve("a.pr");
    String texto = "import regex\nx = regex.\n";
    Files.write(arq, texto.getBytes(StandardCharsets.UTF_8));
    /* a URI como o DefaultUriConverter do LSP4IJ monta: new URI("file", "", abs, null) */
    String uri = new URI("file", "", arq.toAbsolutePath().toString(), null).toString();

    ProcessBuilder pb = new ProcessBuilder(p.getCommands());
    pb.directory(new File(p.getWorkingDirectory()));
    pb.redirectError(ProcessBuilder.Redirect.INHERIT);
    Process proc = pb.start();
    final List<String> avisos = new ArrayList<>();
    LanguageClient cliente = new LanguageClient() {
      public void telemetryEvent(Object o) {}
      public void publishDiagnostics(PublishDiagnosticsParams d) {}
      public void showMessage(MessageParams m) { avisos.add(m.getMessage()); }
      public CompletableFuture<MessageActionItem> showMessageRequest(ShowMessageRequestParams r) { return CompletableFuture.completedFuture(null); }
      public void logMessage(MessageParams m) {}
    };
    try {
      Launcher<LanguageServer> launcher = LSPLauncher.createClientLauncher(cliente, proc.getInputStream(), proc.getOutputStream());
      launcher.startListening();
      LanguageServer srv = launcher.getRemoteProxy();
      InitializeParams ip = new InitializeParams();
      ip.setCapabilities(new ClientCapabilities());
      ip.setInitializationOptions(opts);
      Object init = srv.initialize(ip).get(20, TimeUnit.SECONDS);
      confere("LSP4J: initialize respondido", init != null, null);
      srv.initialized(new InitializedParams());
      srv.getTextDocumentService().didOpen(new DidOpenTextDocumentParams(new TextDocumentItem(uri, "poolscript", 1, texto)));
      CompletionParams cp = new CompletionParams(new TextDocumentIdentifier(uri), new Position(1, 10));
      Either<List<CompletionItem>, CompletionList> r = srv.getTextDocumentService().completion(cp).get(20, TimeUnit.SECONDS);
      int n = r == null ? 0 : (r.isLeft() ? r.getLeft().size() : r.getRight().getItems().size());
      confere("LSP4J: completion em `regex.` responde (" + n + " itens)", n >= 5, n);
      confere("LSP4J: nenhum aviso de motor ausente (o pool foi achado)", avisos.isEmpty(), avisos);
      srv.shutdown().get(10, TimeUnit.SECONDS);
      srv.exit();
    } catch (Exception e) {
      falha("LSP4J: ponta a ponta", e);
    } finally {
      proc.destroy();
    }
  }
}
