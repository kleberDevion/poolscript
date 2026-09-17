package org.jetbrains.plugins.textmate.api;

import java.nio.file.Path;
import java.util.List;

/** Stub do ponto de extensão `com.intellij.textmate.bundleProvider` do plugin
 *  "TextMate Bundles" do IDEA (módulo intellij.textmate). Só a assinatura: em
 *  runtime a classe real do IDEA assume. */
public interface TextMateBundleProvider {
  List<PluginBundle> getBundles();

  /** Nome do bundle + diretório dele no disco. Classe aninhada da interface
   *  (nome binário TextMateBundleProvider$PluginBundle, igual ao real). */
  final class PluginBundle {
    private final String name;
    private final Path path;
    public PluginBundle(String name, Path path) { this.name = name; this.path = path; }
    public String getName() { return name; }
    public Path getPath() { return path; }
  }
}
