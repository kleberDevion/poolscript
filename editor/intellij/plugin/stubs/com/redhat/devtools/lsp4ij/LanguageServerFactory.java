package com.redhat.devtools.lsp4ij;

import com.intellij.openapi.project.Project;
import com.redhat.devtools.lsp4ij.server.StreamConnectionProvider;

/** Stub do ponto de extensão `com.redhat.devtools.lsp4ij.server` (LSP4IJ).
 *  Só o único método abstrato da interface real; os demais são `default` e
 *  vêm dela em runtime. Assinatura do javap do lsp4ij-0.21.0.jar. */
public interface LanguageServerFactory {
  StreamConnectionProvider createConnectionProvider(Project project);
}
