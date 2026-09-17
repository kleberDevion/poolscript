package poolscript.icons;

import com.intellij.ide.FileIconProvider;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.util.IconLoader;
import com.intellij.openapi.vfs.VirtualFile;
import javax.swing.Icon;

/** Ícone do arquivo da PoolScript: a extensão é UMA, `.pr`, e o ícone é a logo
 *  da linguagem — a mesma do VS Code e do tipo MIME do sistema
 *  (`editor/vscode/images/arquivo.png`), reduzida pra `resources/icons/`.
 *  Havia três ícones desenhados, um por extensão antiga (.ps, .p, .psl); as
 *  extensões saíram da linguagem e o desenho saiu junto.
 *
 *  PNG em dois tamanhos: o IDEA escolhe o `@2x` em tela HiDPI sozinho. */
public class PoolIconProvider implements FileIconProvider {
  private static final Icon PR = IconLoader.getIcon("/icons/poolscript_pr.png", PoolIconProvider.class);

  @Override
  public Icon getIcon(VirtualFile file, int flags, Project project) {
    String ext = file.getExtension();
    if (ext == null) return null;
    return ext.equalsIgnoreCase("pr") ? PR : null;
  }
}
