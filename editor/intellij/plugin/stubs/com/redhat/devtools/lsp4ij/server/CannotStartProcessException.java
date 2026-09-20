package com.redhat.devtools.lsp4ij.server;

public class CannotStartProcessException extends LanguageServerException {
  public CannotStartProcessException(String message) { super(message); }
  public CannotStartProcessException(Exception cause) { super(cause); }
}
