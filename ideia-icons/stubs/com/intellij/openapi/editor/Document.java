package com.intellij.openapi.editor;
public interface Document {
  String getText();
  int getLineNumber(int offset);
  int getLineStartOffset(int line);
  int getLineEndOffset(int line);
  void insertString(int offset, CharSequence s);
  void replaceString(int start, int end, CharSequence s);
}
