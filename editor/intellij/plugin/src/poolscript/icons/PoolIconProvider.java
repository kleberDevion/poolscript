package poolscript.icons;

import com.intellij.ide.FileIconProvider;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.util.IconLoader;
import com.intellij.openapi.vfs.VirtualFile;
import javax.swing.Icon;

/** Ícone dos arquivos da PoolScript, um por extensão:
 *  .ps "PS" azul #2196F7, .p "P" azul #2196F7, .psl "&lt;PSL/&gt;" vermelho #F44336.
 *
 *  Os SVG são CONTORNO, não &lt;text&gt;: o renderizador do IntelliJ não garante
 *  fonte, e o que não resolve sai com outra métrica ou vazio. */
public class PoolIconProvider implements FileIconProvider {
  private static final Icon PS  = IconLoader.getIcon("/icons/poolscript_ps.svg",  PoolIconProvider.class);
  private static final Icon P   = IconLoader.getIcon("/icons/poolscript_p.svg",   PoolIconProvider.class);
  private static final Icon PSL = IconLoader.getIcon("/icons/poolscript_psl.svg", PoolIconProvider.class);

  @Override
  public Icon getIcon(VirtualFile file, int flags, Project project) {
    String ext = file.getExtension();
    if (ext == null) return null;
    switch (ext.toLowerCase()) {
      case "ps":  return PS;
      case "p":   return P;
      case "psl": return PSL;
      default:    return null;
    }
  }
}
