import * as vscode from "vscode";
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  TransportKind,
} from "vscode-languageclient/node";

let client: LanguageClient | undefined;

export function activate(context: vscode.ExtensionContext): void {
  const config = vscode.workspace.getConfiguration("poolscript");

  if (!config.get<boolean>("lsp.enabled", true)) {
    return;
  }

  const pythonPath = config.get<string>("pythonPath", "python");

  const serverOptions: ServerOptions = {
    command: pythonPath,
    args: ["-m", "poolscript.lsp.server"],
    transport: TransportKind.stdio,
  };

  const clientOptions: LanguageClientOptions = {
    documentSelector: [{ scheme: "file", language: "poolscript" }],
    synchronize: {
      fileEvents: vscode.workspace.createFileSystemWatcher("**/*.ps"),
    },
  };

  client = new LanguageClient(
    "poolscriptLanguageServer",
    "PoolScript Language Server",
    serverOptions,
    clientOptions
  );

  client.start().then(
    () => {
      // ok — servidor de pé
    },
    (err: unknown) => {
      vscode.window.showWarningMessage(
        `PoolScript: não consegui iniciar o language server (python="${pythonPath}"). ` +
          `Verifique se 'pip install -e ".[lsp]"' foi rodado e se poolscript.pythonPath está correto. ` +
          `Detalhe: ${String(err)}`
      );
    }
  );

  context.subscriptions.push({
    dispose: () => {
      void client?.stop();
    },
  });
}

export function deactivate(): Thenable<void> | undefined {
  return client?.stop();
}
