package com.intellij.codeInsight.editorActions.enter;
public interface EnterHandlerDelegate {
  enum Result { Continue, DefaultForceIndent, DefaultSkipIndent, Stop }
  Result preprocessEnter(com.intellij.psi.PsiFile file,
                         com.intellij.openapi.editor.Editor editor,
                         com.intellij.openapi.util.Ref<Integer> caretOffset,
                         com.intellij.openapi.util.Ref<Integer> caretAdvance,
                         com.intellij.openapi.actionSystem.DataContext dataContext,
                         com.intellij.openapi.editor.actionSystem.EditorActionHandler originalHandler);
  Result postProcessEnter(com.intellij.psi.PsiFile file,
                          com.intellij.openapi.editor.Editor editor,
                          com.intellij.openapi.actionSystem.DataContext dataContext);
}
