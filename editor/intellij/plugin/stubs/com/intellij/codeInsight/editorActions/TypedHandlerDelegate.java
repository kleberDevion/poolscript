package com.intellij.codeInsight.editorActions;
import com.intellij.openapi.editor.Editor;
import com.intellij.openapi.fileTypes.FileType;
import com.intellij.openapi.project.Project;
import com.intellij.psi.PsiFile;
public abstract class TypedHandlerDelegate {
  public enum Result { STOP, CONTINUE, DEFAULT }
  public Result beforeCharTyped(char c, Project project, Editor editor, PsiFile file, FileType fileType) {
    return Result.CONTINUE;
  }
}
