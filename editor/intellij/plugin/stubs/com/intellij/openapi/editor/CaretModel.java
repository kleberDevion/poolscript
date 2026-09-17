package com.intellij.openapi.editor;
public interface CaretModel {
  int getOffset();
  void moveToOffset(int offset);
}
