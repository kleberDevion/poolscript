package poolscript.icons;

import com.intellij.openapi.application.PathManager;
import org.jetbrains.plugins.textmate.api.TextMateBundleProvider;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UncheckedIOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Registra o realce da PoolScript no IDEA SOZINHO, no boot.
 *
 * O IDEA só colore com bundle TextMate que esteja registrado: ou o usuário
 * adiciona na mão (Settings > Editor > TextMate Bundles > +), ou um plugin
 * entrega pelo ponto de extensão `com.intellij.textmate.bundleProvider`. Era
 * o passo manual — e sem ele o `.pr` abria sem cor, por mais que o jar estivesse
 * instalado. Agora o bundle vai DENTRO do jar (`textmate/PoolScript.tmbundle`,
 * copiado pelo build.sh) e este provedor:
 *
 *   1. extrai o bundle pro diretório de sistema do IDEA
 *      (<system>/poolscript/PoolScript.tmbundle), sobrescrevendo — assim um
 *      jar novo com gramática nova vale no próximo boot, sem cache velho;
 *   2. devolve o caminho pro IDEA registrar, como faz com os bundles de fábrica.
 *
 * O ponto de extensão pede um Path no disco (o leitor lista diretórios), por
 * isso a extração; ler direto de dentro do jar não serve.
 *
 * A lista de arquivos (`textmate/lista.txt`) é gerada pelo build.sh a partir do
 * bundle — nada de nome fixo aqui: o bundle muda, a lista acompanha.
 */
public class PoolBundle implements TextMateBundleProvider {
  private static final String RAIZ  = "/textmate/";
  private static final String LISTA = RAIZ + "lista.txt";
  private static final String NOME  = "PoolScript";

  @Override
  public List<PluginBundle> getBundles() {
    try {
      List<String> arquivos = lista();
      if (arquivos.isEmpty()) {
        throw new IOException("jar sem " + LISTA + " — o build.sh nao embutiu o bundle");
      }
      Path destino = Paths.get(PathManager.getSystemPath(), "poolscript");
      for (String rel : arquivos) {
        Path alvo = destino.resolve(rel);
        Files.createDirectories(alvo.getParent());
        try (InputStream in = PoolBundle.class.getResourceAsStream(RAIZ + rel)) {
          if (in == null) throw new IOException("falta no jar: " + RAIZ + rel);
          Files.copy(in, alvo, StandardCopyOption.REPLACE_EXISTING);
        }
      }
      return Collections.singletonList(new PluginBundle(NOME, destino.resolve(NOME + ".tmbundle")));
    } catch (IOException e) {
      // sobe pro log do IDEA (idea.log) em vez de sumir calado sem realce
      throw new UncheckedIOException("PoolScript: nao consegui extrair o bundle TextMate", e);
    }
  }

  private static List<String> lista() throws IOException {
    List<String> r = new ArrayList<>();
    try (InputStream in = PoolBundle.class.getResourceAsStream(LISTA)) {
      if (in == null) return r;
      BufferedReader br = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8));
      String linha;
      while ((linha = br.readLine()) != null) {
        linha = linha.trim();
        if (!linha.isEmpty()) r.add(linha);
      }
    }
    return r;
  }
}
