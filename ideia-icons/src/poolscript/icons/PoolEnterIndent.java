package poolscript.icons;

import com.intellij.codeInsight.editorActions.enter.EnterHandlerDelegate;
import com.intellij.openapi.actionSystem.DataContext;
import com.intellij.openapi.editor.Document;
import com.intellij.openapi.editor.Editor;
import com.intellij.openapi.editor.actionSystem.EditorActionHandler;
import com.intellij.openapi.util.Ref;
import com.intellij.openapi.vfs.VirtualFile;
import com.intellij.psi.PsiFile;

/**
 * Indentação no Enter pra PoolScript (.ps/.psl/.p) — mesmo comportamento do
 * VS Code, que o suporte TextMate do IDEA não dá:
 *   - linha anterior termina com aberto ({ ( [) -> nova linha entra com +4
 *   - Enter entre { e }                        -> } desce alinhado, cursor +4
 *   - linha nova começando com fechamento      -> desalinha 4
 *   - resto                                    -> copia a indentação de cima
 */
public class PoolEnterIndent implements EnterHandlerDelegate {
  private static final int PASSO = 4;

  @Override
  public Result preprocessEnter(PsiFile file, Editor editor, Ref<Integer> caretOffset,
                                Ref<Integer> caretAdvance, DataContext dataContext,
                                EditorActionHandler originalHandler) {
    return Result.Continue;
  }

  @Override
  public Result postProcessEnter(PsiFile file, Editor editor, DataContext ctx) {
    VirtualFile vf = file.getVirtualFile();
    if (vf == null) return Result.Continue;
    String ext = vf.getExtension();
    if (ext == null) return Result.Continue;
    ext = ext.toLowerCase();
    if (!ext.equals("ps") && !ext.equals("psl") && !ext.equals("p")) return Result.Continue;

    Document doc = editor.getDocument();
    String texto = doc.getText();
    int offset = editor.getCaretModel().getOffset();
    if (offset > texto.length()) return Result.Continue;
    int linha = doc.getLineNumber(offset);
    if (linha == 0) return Result.Continue;

    // linha anterior não-vazia
    int lp = linha - 1;
    String prev = "";
    while (lp >= 0) {
      prev = texto.substring(doc.getLineStartOffset(lp), doc.getLineEndOffset(lp));
      if (!prev.trim().isEmpty()) break;
      lp--;
    }
    String prevTrim = prev.trim();
    int base = indentDe(prev);
    boolean abriu = prevTrim.endsWith("{") || prevTrim.endsWith("(") || prevTrim.endsWith("[");

    int inicioLinha = doc.getLineStartOffset(linha);
    int fimLinha = doc.getLineEndOffset(linha);
    String resto = texto.substring(Math.min(offset, fimLinha), fimLinha).trim();

    if (abriu && (resto.startsWith("}") || resto.startsWith(")") || resto.startsWith("]"))) {
      // Enter entre abre e fecha: fecha desce alinhado, cursor fica no meio
      String corpo = espacos(base + PASSO);
      doc.replaceString(inicioLinha, offset, corpo);
      int caret = inicioLinha + corpo.length();
      doc.insertString(caret, "\n" + espacos(base));
      editor.getCaretModel().moveToOffset(caret);
      return Result.Stop;
    }

    int alvo;
    if (abriu) alvo = base + PASSO;
    else if (resto.startsWith("}") || resto.startsWith(")") || resto.startsWith("]"))
      alvo = Math.max(base - PASSO, 0);
    else alvo = base;

    // normaliza o espaço em branco do começo da linha nova pro alvo
    int fimWs = inicioLinha;
    while (fimWs < texto.length() && (texto.charAt(fimWs) == ' ' || texto.charAt(fimWs) == '\t'))
      fimWs++;
    String atual = texto.substring(inicioLinha, fimWs);
    String desejado = espacos(alvo);
    if (!atual.equals(desejado)) {
      doc.replaceString(inicioLinha, fimWs, desejado);
    }
    editor.getCaretModel().moveToOffset(inicioLinha + desejado.length());
    return Result.Stop;
  }

  private static int indentDe(String linha) {
    int n = 0;
    while (n < linha.length() && linha.charAt(n) == ' ') n++;
    return n;
  }

  private static String espacos(int n) {
    StringBuilder sb = new StringBuilder();
    for (int i = 0; i < n; i++) sb.append(' ');
    return sb.toString();
  }
}
