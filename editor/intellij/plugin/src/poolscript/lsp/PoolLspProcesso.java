package poolscript.lsp;

import com.intellij.openapi.project.Project;
import com.intellij.openapi.vfs.VirtualFile;
import com.redhat.devtools.lsp4ij.server.CannotStartProcessException;
import com.redhat.devtools.lsp4ij.server.ProcessStreamConnectionProvider;

import java.nio.file.Paths;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/** O processo do servidor: `poolscript-lsp --stdio`, com o caminho ABSOLUTO
 *  resolvido pelo plugin, a pasta do projeto como pasta de trabalho, o PATH
 *  reforçado e o `pool` em `initializationOptions`. */
public class PoolLspProcesso extends ProcessStreamConnectionProvider {
  private final String pool;

  PoolLspProcesso(Project projeto) {
    super(comandos(), PoolLspFactory.pastaDoProjeto(projeto));
    setUserEnvironmentVariables(Collections.singletonMap("PATH", PoolLspFactory.pathReforcado()));
    pool = PoolLspFactory.acha("pool", System.getenv("PATH"));
  }

  static List<String> comandos() {
    String lsp = PoolLspFactory.acha("poolscript-lsp", System.getenv("PATH"));
    return Arrays.asList(lsp == null ? "poolscript-lsp" : lsp, "--stdio");
  }

  /** `{"pool": "/usr/local/bin/pool"}` — a única coisa que o servidor lê do
   *  cliente. Sem o pool achado vai vazio, e é o SERVIDOR que avisa. */
  @Override
  public Object getInitializationOptions(VirtualFile rootUri) {
    return pool == null ? Collections.emptyMap() : Collections.singletonMap("pool", pool);
  }

  /** Sem o `poolscript-lsp` achado, a partida falha DIZENDO onde procurou —
   *  no console do LSP4IJ — em vez do servidor nem subir, calado. */
  @Override
  public void start() throws CannotStartProcessException {
    List<String> c = getCommands();
    if (c == null || c.isEmpty() || !Paths.get(c.get(0)).isAbsolute())
      throw new CannotStartProcessException("PoolScript: nao achei `poolscript-lsp` no PATH do IDEA ("
          + System.getenv("PATH") + ") nem em /usr/local/bin, ~/.local/bin ou ~/.poolscript/bin — rode "
          + "`sudo make install` no repositorio da linguagem e reinicie o servidor.");
    super.start();
  }
}
