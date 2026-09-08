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
const { workspace, window, commands } = require('vscode');
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');

let cliente;
let terminal;

// O binário da linguagem. O servidor pergunta TUDO pra ele — módulos, tokens,
// diagnóstico — então apontar pro `pool` errado faz o completion descrever um
// motor que não é o que o usuário roda.
function poolBin() {
  const dado = workspace.getConfiguration('poolscript').get('pool');
  return (typeof dado === 'string' && dado.trim()) ? dado.trim() : 'pool';
}

/* Aspas simples ao redor, e aspas simples de dentro escapadas: o caminho vai
 * pro shell, e uma pasta com espaço ou apóstrofo viraria dois argumentos. */
function pro_shell(s) {
  return "'" + String(s).split("'").join("'\\''") + "'";
}

/* Roda o arquivo do editor no terminal integrado.
 *
 * UM terminal só, reaproveitado: abrir um por execução enche o painel e some
 * com a saída anterior, que é justamente o que se quer comparar. Se o usuário
 * fechou o dele, o `exitStatus` acusa e a gente cria outro.
 *
 * Salva antes de rodar — rodar a versão em disco enquanto o editor mostra
 * outra é a forma mais rápida de perseguir um erro que já foi corrigido. */
async function rodarArquivo() {
  const ed = window.activeTextEditor;
  if (!ed) {
    window.showWarningMessage('PoolScript: nenhum arquivo aberto pra rodar.');
    return;
  }
  const doc = ed.document;
  if (doc.languageId !== 'poolscript' && doc.languageId !== 'poolscript-psl') {
    window.showWarningMessage('PoolScript: este arquivo não é .ps, .p nem .psl.');
    return;
  }
  if (doc.isUntitled) {
    window.showWarningMessage('PoolScript: salve o arquivo antes de rodar.');
    return;
  }
  if (doc.isDirty) await doc.save();

  if (!terminal || terminal.exitStatus !== undefined) {
    terminal = window.createTerminal({ name: 'PoolScript' });
  }
  terminal.show(true);
  const arq = doc.uri.fsPath;
  /* `cd` na pasta do arquivo: caminho relativo do programa (`os.readFile`,
   * `import` por caminho) sai do diretório atual, então rodar da raiz do
   * projeto acharia arquivo diferente do que o programa espera. */
  terminal.sendText('cd ' + pro_shell(path.dirname(arq)));
  terminal.sendText(pro_shell(poolBin()) + ' ' + pro_shell(path.basename(arq)));
}

function activate(context) {
  /* Os comandos ficam FORA do `if` do LSP: quem desliga o servidor de
   * linguagem não está pedindo pra perder o botão de rodar. */
  context.subscriptions.push(commands.registerCommand('poolscript.rodar', rodarArquivo));

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
