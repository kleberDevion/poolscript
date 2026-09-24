package jinga.lsp;

import com.intellij.openapi.project.Project;
import com.intellij.openapi.vfs.VirtualFile;
import com.redhat.devtools.lsp4ij.server.CannotStartProcessException;
import com.redhat.devtools.lsp4ij.server.ProcessStreamConnectionProvider;

import java.nio.file.Paths;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/** O processo do servidor: `jinga-lsp --stdio`, com o caminho ABSOLUTO
 *  resolvido pelo plugin, a pasta do projeto como pasta de trabalho, o PATH
 *  reforçado e o motor (`jinga`) em `initializationOptions`. */
public class JingaLspProcesso extends ProcessStreamConnectionProvider {
  private final String motor;

  JingaLspProcesso(Project projeto) {
    super(comandos(), JingaLspFactory.pastaDoProjeto(projeto));
    setUserEnvironmentVariables(Collections.singletonMap("PATH", JingaLspFactory.pathReforcado()));
    motor = JingaLspFactory.achaQualquer(JingaLspFactory.MOTOR, System.getenv("PATH"));
  }

  /** `jinga-lsp` primeiro; `poolscript-lsp` (o atalho antigo) de reserva. */
  static List<String> comandos() {
    String lsp = JingaLspFactory.achaQualquer(JingaLspFactory.LSP, System.getenv("PATH"));
    return Arrays.asList(lsp == null ? "jinga-lsp" : lsp, "--stdio");
  }

  /** `{"pool": "/usr/local/bin/jinga"}` — a única coisa que o servidor lê do
   *  cliente; a chave é a que o server.js lê. Sem o motor achado vai vazio, e
   *  é o SERVIDOR que avisa. */
  @Override
  public Object getInitializationOptions(VirtualFile rootUri) {
    return motor == null ? Collections.emptyMap() : Collections.singletonMap("pool", motor);
  }

  /** Sem o `jinga-lsp` achado, a partida falha DIZENDO onde procurou —
   *  no console do LSP4IJ — em vez do servidor nem subir, calado. */
  @Override
  public void start() throws CannotStartProcessException {
    List<String> c = getCommands();
    if (c == null || c.isEmpty() || !Paths.get(c.get(0)).isAbsolute())
      throw new CannotStartProcessException("Jinga: nao achei `jinga-lsp` (nem `poolscript-lsp`) no PATH do IDEA ("
          + System.getenv("PATH") + ") nem em /usr/local/bin, ~/.local/bin, ~/.jinga/bin ou ~/.poolscript/bin — rode "
          + "`sudo make install` no repositorio da linguagem e reinicie o servidor.");
    super.start();
  }
}
