package com.redhat.devtools.lsp4ij.server;

import com.intellij.openapi.vfs.VirtualFile;
import java.io.InputStream;
import java.io.OutputStream;

/** Stub: só as assinaturas que o plugin usa (javap do lsp4ij-0.21.0.jar). */
public interface StreamConnectionProvider {
  void start() throws CannotStartProcessException;
  InputStream getInputStream();
  OutputStream getOutputStream();
  void stop();
  /** O que vai em `initializationOptions` do `initialize`; um Map vira JSON. */
  default Object getInitializationOptions(VirtualFile rootUri) { return null; }
}
