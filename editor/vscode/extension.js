/*
 * Cliente LSP da PoolScript para o VS Code.
 *
 * Este arquivo NÃO é o suporte da linguagem — ele é o soquete. Todo o
 * completion, hover, diagnóstico e realce vive em `lsp/servidor.ps`, escrito
 * em PoolScript e rodado pelo `pool`. O VS Code só carrega extensão cujo ponto
 * de entrada é JavaScript (regra do editor), então sobra este lançador.
 *
 * Se um dia o servidor ganhar uma capacidade nova, nada aqui muda.
 *
 * O comando padrão é `poolscript-lsp`, instalado em /usr/local/bin. Pra rodar
 * direto do repositório (útil enquanto se mexe no servidor):
 *
 *     "poolscript.lsp.comando": ["pool", "/caminho/do/repo/lsp/servidor.ps"]
 *
 * E `"poolscript.lsp.ativo": false` desliga — aí sobra só o realce da
 * gramática, que é declarativo e não depende deste arquivo.
 */
const { workspace, window } = require('vscode');
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');

let cliente;

function comando() {
  const cfg = workspace.getConfiguration('poolscript');
  const dado = cfg.get('lsp.comando');
  if (Array.isArray(dado) && dado.length > 0) {
    return { command: dado[0], args: dado.slice(1) };
  }
  return { command: 'poolscript-lsp', args: [] };
}

function activate(context) {
  if (!workspace.getConfiguration('poolscript').get('lsp.ativo')) return;
  const { command, args } = comando();

  const servidor = {
    run:   { command, args, transport: TransportKind.stdio },
    debug: { command, args, transport: TransportKind.stdio },
  };

  const cliente_opts = {
    documentSelector: [
      { scheme: 'file', language: 'poolscript' },
      { scheme: 'file', language: 'poolscript-psl' },
    ],
    // O servidor lê o documento que o editor manda; não precisa observar disco.
    synchronize: { fileEvents: workspace.createFileSystemWatcher('**/*.{ps,psl,p}') },
    outputChannelName: 'PoolScript',
  };

  cliente = new LanguageClient('poolscript', 'PoolScript', servidor, cliente_opts);

  cliente.start().catch((e) => {
    // Falhar calado deixaria o usuário sem completion sem saber por quê.
    window.showErrorMessage(
      `PoolScript: não consegui iniciar o servidor (${command}). ` +
      `Instale com \`make install\` no repositório, ou aponte ` +
      `\`poolscript.lsp.comando\` para o lsp/servidor.ps. Detalhe: ${e.message}`
    );
  });

  context.subscriptions.push({ dispose: () => cliente && cliente.stop() });
}

function deactivate() {
  return cliente ? cliente.stop() : undefined;
}

module.exports = { activate, deactivate };
