package poolscript.icons;

import com.intellij.codeInsight.editorActions.TypedHandlerDelegate;
import com.intellij.openapi.editor.Document;
import com.intellij.openapi.editor.Editor;
import com.intellij.openapi.fileTypes.FileType;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.vfs.VirtualFile;
import com.intellij.psi.PsiFile;

/**
 * Auto-fechamento CORRETO de bracket/aspas para PoolScript (.ps/.psl/.p),
 * feito no plugin porque o IDEA não faz type-over em arquivo TextMate:
 *
 *   - digitar '{' '(' '[' insere o par e deixa o cursor no meio (uma vez só)
 *   - digitar '}' ')' ']' quando o próximo caractere já é ele: PASSA POR CIMA
 *     em vez de inserir outro — mata o {}} .
 *   - aspas: fecha o par; sobre a aspa de fechamento, passa por cima.
 *
 * O bundle TextMate tem smartTypingPairs VAZIO, então este handler é a ÚNICA
 * fonte de auto-close — sem conflito, sem duplicação.
 */
public class PoolBraces extends TypedHandlerDelegate {

  private static boolean ehPoolScript(PsiFile file) {
    if (file == null) return false;
    VirtualFile vf = file.getVirtualFile();
    if (vf == null) return false;
    String ext = vf.getExtension();
    if (ext == null) return false;
    ext = ext.toLowerCase();
    return ext.equals("ps") || ext.equals("psl") || ext.equals("p");
  }

  private static char fechaDe(char c) {
    switch (c) {
      case '{': return '}';
      case '(': return ')';
      case '[': return ']';
      default:  return 0;
    }
  }

  @Override
  public Result beforeCharTyped(char c, Project project, Editor editor, PsiFile file, FileType fileType) {
    if (!ehPoolScript(file)) return Result.CONTINUE;

    Document doc = editor.getDocument();
    int off = editor.getCaretModel().getOffset();
    CharSequence txt = doc.getCharsSequence();
    char proximo = off < txt.length() ? txt.charAt(off) : '\0';

    // type-over: fechar sobre o próprio fechamento já presente
    if ((c == '}' || c == ')' || c == ']') && proximo == c) {
      editor.getCaretModel().moveToOffset(off + 1);
      return Result.STOP;
    }

    // abre bracket -> insere o par, cursor no meio
    char fecha = fechaDe(c);
    if (fecha != 0) {
      doc.insertString(off, String.valueOf(c) + fecha);
      editor.getCaretModel().moveToOffset(off + 1);
      return Result.STOP;
    }

    // aspas: type-over sobre a de fechamento, senão abre o par
    if (c == '"' || c == '\'') {
      if (proximo == c) {
        editor.getCaretModel().moveToOffset(off + 1);
        return Result.STOP;
      }
      // só auto-fecha se o cursor está antes de espaço, fechamento ou fim de linha
      if (proximo == '\0' || proximo == '\n' || proximo == ' ' || proximo == '\t'
          || proximo == ')' || proximo == ']' || proximo == '}' || proximo == ',') {
        doc.insertString(off, String.valueOf(c) + c);
        editor.getCaretModel().moveToOffset(off + 1);
        return Result.STOP;
      }
    }
    return Result.CONTINUE;
  }
}
