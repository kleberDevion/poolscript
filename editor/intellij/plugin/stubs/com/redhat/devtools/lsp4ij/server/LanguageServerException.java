package com.redhat.devtools.lsp4ij.server;

public class LanguageServerException extends RuntimeException {
  public LanguageServerException(String message) { super(message); }
  public LanguageServerException(Throwable cause) { super(cause); }
}
