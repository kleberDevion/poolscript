package jinga.lsp;

import com.intellij.notification.NotificationGroupManager;
import com.intellij.notification.NotificationType;
import com.intellij.openapi.project.Project;
import com.redhat.devtools.lsp4ij.LanguageServerFactory;
import com.redhat.devtools.lsp4ij.LanguageServersRegistry;
import com.redhat.devtools.lsp4ij.server.StreamConnectionProvider;
import com.redhat.devtools.lsp4ij.server.definition.LanguageServerDefinition;
import com.redhat.devtools.lsp4ij.server.definition.launching.UserDefinedLanguageServerDefinition;

import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

/**
 * Declara o servidor LSP da Jinga ao LSP4IJ — ponto de extensão
 * `com.redhat.devtools.lsp4ij.server` (ver jinga-lsp4ij.xml).
 *
 * POR QUE: o servidor era cadastrado à mão em Settings > Language Servers, e o
 * cadastro da máquina ainda mapeava `*.ps/*.psl/*.p` — a linguagem migrou pra
 * `.pr` e o servidor nunca era ligado; o que sobrava era a completion de
 * palavras do próprio IDEA ("só sugere texto que já está escrito"). Declarado
 * pelo plugin, o mapeamento `*.pr` vai junto com a gramática, e não há passo
 * manual pra apodrecer.
 *
 * Os caminhos do `jinga-lsp` e do `jinga` são resolvidos AQUI, em
 * absoluto: o PATH do processo do IDEA (aberto pelo desktop) não é o do
 * terminal. O motor vai pro servidor em `initializationOptions` — sem isso,
 * com o motor fora do PATH, ele respondia vazio sem um aviso.
 *
 * Os nomes antigos (`poolscript-lsp`, `pool`, `~/.poolscript/bin`) seguem como
 * reserva: o instalador antigo deixou esses no disco, e o `make install` ainda
 * cria o `poolscript-lsp` como atalho.
 */
public class JingaLspFactory implements LanguageServerFactory {
  /** O servidor: o nome novo primeiro; o antigo, ainda instalado como atalho, de reserva. */
  static final String[] LSP   = { "jinga-lsp", "poolscript-lsp" };
  /** O motor, na mesma ordem. */
  static final String[] MOTOR = { "jinga", "pool" };

  private static boolean avisou = false;

  @Override
  public StreamConnectionProvider createConnectionProvider(Project projeto) {
    avisaSeDuplicado(projeto);
    return new JingaLspProcesso(projeto);
  }

  /** Onde procurar um executável: as entradas do PATH do IDEA e, depois, os
   *  lugares onde o `make install` e o instalador põem o motor. */
  static List<String> pastasDeBusca(String path) {
    List<String> pastas = new ArrayList<>();
    if (path != null) {
      for (String p : path.split(File.pathSeparator)) if (!p.isEmpty() && !pastas.contains(p)) pastas.add(p);
    }
    String home = System.getProperty("user.home");
    String[] extras = { "/usr/local/bin", "/usr/bin",
                        home + "/.local/bin", home + "/bin",
                        home + "/.jinga/bin", home + "/.poolscript/bin" };
    for (String p : extras) if (!pastas.contains(p)) pastas.add(p);
    return pastas;
  }

  /** O primeiro `nome` executável nas pastas de busca, absoluto; null se não há. */
  static String acha(String nome, String path) {
    for (String pasta : pastasDeBusca(path)) {
      Path p = Paths.get(pasta, nome);
      if (Files.isRegularFile(p) && Files.isExecutable(p)) return p.toAbsolutePath().toString();
    }
    return null;
  }

  /** O primeiro dos `nomes` que existe, na ordem dada — `jinga-lsp` em qualquer
   *  pasta antes de `poolscript-lsp` em qualquer pasta; null se nenhum. */
  static String achaQualquer(String[] nomes, String path) {
    for (String nome : nomes) {
      String r = acha(nome, path);
      if (r != null) return r;
    }
    return null;
  }

  /** O PATH que o servidor recebe: o do IDEA mais as pastas de busca — o
   *  `jinga-lsp` precisa achar o `node`, e o servidor procura o motor no PATH. */
  static String pathReforcado() {
    return String.join(File.pathSeparator, pastasDeBusca(System.getenv("PATH")));
  }

  /** Pasta de trabalho do servidor: a do projeto (um processo por projeto;
   *  é onde o servidor acha a `docs/` quando o projeto é o repositório). */
  static String pastaDoProjeto(Project projeto) {
    String base = projeto != null ? projeto.getBasePath() : null;
    if (base != null && new File(base).isDirectory()) return base;
    return System.getProperty("user.home");
  }

  /** Um servidor "Jinga" cadastrado à mão, além deste, sobe DOIS
   *  processos — e o LSP4IJ não acusa. Aviso uma vez por sessão. Qualquer
   *  falha aqui (fora do IDEA, API diferente) só cala o aviso: nunca segura
   *  o servidor. Pega o cadastro com o nome novo e com o antigo. */
  static void avisaSeDuplicado(Project projeto) {
    if (avisou) return;
    try {
      for (LanguageServerDefinition d : LanguageServersRegistry.getInstance().getServerDefinitions()) {
        if (!(d instanceof UserDefinedLanguageServerDefinition)) continue;
        String cmd = ((UserDefinedLanguageServerDefinition) d).getCommandLine();
        if (cmd == null || !(cmd.contains("jinga-lsp") || cmd.contains("poolscript-lsp"))) continue;
        avisou = true;
        NotificationGroupManager.getInstance().getNotificationGroup("Jinga")
          .createNotification("Jinga: ha um servidor '" + d.getDisplayName()
              + "' cadastrado a mao em Settings > Languages & Frameworks > Language Servers. "
              + "O plugin ja registra o servidor da linguagem — apague o cadastro manual, "
              + "senao sobem dois processos.", NotificationType.WARNING)
          .notify(projeto);
        return;
      }
    } catch (Throwable ignorado) {
      /* sem aviso, nunca sem servidor */
    }
  }
}
