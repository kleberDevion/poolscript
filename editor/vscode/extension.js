/*
 * Cliente LSP da PoolScript para o VS Code.
 *
 * Este arquivo é o soquete: quem faz o trabalho é `server.js`, sobre
 * `vscode-languageserver` — a implementação de REFERÊNCIA do protocolo. O
 * cérebro da linguagem continua sendo o motor (`pool --metadata`, `--tokens`,
 * `--check`); nada de lista de método escrita à mão.
 *
 * `"poolscript.lsp.ativo": false` desliga e sobra só o realce da gramática,
 * que é declarativo. `"poolscript.pool"` aponta o binário quando ele não está
 * no PATH.
 */
const path = require('path');
const { workspace, window } = require('vscode');
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');

let cliente;

// O binário da linguagem. O servidor pergunta TUDO pra ele — módulos, tokens,
// diagnóstico — então apontar pro `pool` errado faz o completion descrever um
// motor que não é o que o usuário roda.
function poolBin() {
  const dado = workspace.getConfiguration('poolscript').get('pool');
  return (typeof dado === 'string' && dado.trim()) ? dado.trim() : 'pool';
}

function activate(context) {
  if (!workspace.getConfiguration('poolscript').get('lsp.ativo')) return;

  // O servidor roda no MESMO Node do VS Code, pelo módulo `vscode-languageserver`.
  // Antes era um processo externo (`poolscript-lsp`, PoolScript implementando o
  // protocolo à mão): quem não tivesse feito `make install` ficava sem nada, e o
  // protocolo era metade do trabalho pra um resultado pior.
  const servidor = {
    run:   { module: path.join(__dirname, 'server.js'), transport: TransportKind.stdio },
    debug: { module: path.join(__dirname, 'server.js'), transport: TransportKind.stdio,
             options: { execArgv: ['--nolazy', '--inspect=6009'] } },
  };

  const cliente_opts = {
    documentSelector: [
      { scheme: 'file', language: 'poolscript' },
      { scheme: 'file', language: 'poolscript-psl' },
    ],
    synchronize: { fileEvents: workspace.createFileSystemWatcher('**/*.{ps,psl,p}') },
    outputChannelName: 'PoolScript',
    initializationOptions: { pool: poolBin() },
  };

  cliente = new LanguageClient('poolscript', 'PoolScript', servidor, cliente_opts);

  cliente.start().catch((e) => {
    // Falhar calado deixaria o usuário sem completion sem saber por quê.
    window.showErrorMessage(
      `PoolScript: o servidor de linguagem não subiu. Confira se o \`pool\` está ` +
      `no PATH (ou aponte \`poolscript.pool\`). Detalhe: ${e.message}`
    );
  });

  context.subscriptions.push({ dispose: () => cliente && cliente.stop() });
}

function deactivate() {
  return cliente ? cliente.stop() : undefined;
}

module.exports = { activate, deactivate };
