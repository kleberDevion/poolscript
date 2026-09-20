package com.redhat.devtools.lsp4ij.server;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;
import java.util.Map;

/** Stub da classe que sobe o servidor como processo (javap do
 *  lsp4ij-0.21.0.jar). No binário real nenhum método é abstrato: aqui
 *  eles ficam concretos e vazios, pra uma subclasse que só chama
 *  `super(comandos, pasta)` compilar como compila contra a real. */
public abstract class ProcessStreamConnectionProvider implements StreamConnectionProvider {
  public ProcessStreamConnectionProvider() {}
  public ProcessStreamConnectionProvider(List<String> commands) {}
  public ProcessStreamConnectionProvider(List<String> commands, String workingDirectory) {}
  public void start() throws CannotStartProcessException {}
  public InputStream getInputStream() { return null; }
  public OutputStream getOutputStream() { return null; }
  public void stop() {}
  public List<String> getCommands() { return null; }
  public void setCommands(List<String> commands) {}
  public String getWorkingDirectory() { return null; }
  public void setWorkingDirectory(String workingDirectory) {}
  public void setUserEnvironmentVariables(Map<String, String> env) {}
}
