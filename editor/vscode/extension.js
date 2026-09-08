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
const net = require('net');
const { spawn } = require('child_process');
const vscode = require('vscode');
const { workspace, window, commands, debug, languages } = vscode;
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');

let cliente;
let terminal;
let saidaDebug;

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

/* Começa uma sessão de depuração no arquivo do editor.
 *
 * Existe pra ficar ao lado do "Rodar" no topo do editor: quem está com o
 * arquivo aberto quer escolher entre rodar e depurar ali, não decorar que o
 * atalho global é F5. Salva antes, pelo mesmo motivo do `rodarArquivo` —
 * depurar a versão em disco enquanto o editor mostra outra é perseguir um erro
 * que já foi corrigido. */
async function depurarArquivo() {
  const ed = window.activeTextEditor;
  if (!ed) {
    window.showWarningMessage('PoolScript: nenhum arquivo aberto pra depurar.');
    return;
  }
  const doc = ed.document;
  if (doc.languageId !== 'poolscript' && doc.languageId !== 'poolscript-psl') {
    window.showWarningMessage('PoolScript: este arquivo não é .ps, .p nem .psl.');
    return;
  }
  if (doc.isUntitled) {
    window.showWarningMessage('PoolScript: salve o arquivo antes de depurar.');
    return;
  }
  if (doc.isDirty) await doc.save();

  const arq = doc.uri.fsPath;
  const pasta = workspace.getWorkspaceFolder(doc.uri);
  await debug.startDebugging(pasta, {
    type: 'poolscript',
    request: 'launch',
    name: 'Depurar ' + path.basename(arq),
    programa: arq,
    cwd: path.dirname(arq)
  });
}

/* ── `Rodar | Depurar` em cima do ponto de entrada ──────────────────────────
 *
 * O mesmo lugar em que o Java põe o `Run | Debug` acima do `main`: quem abriu o
 * arquivo age ali, sem procurar botão no topo nem decorar atalho.
 *
 * A âncora é o `if __name__ == "main" {`, que é onde um programa PoolScript
 * começa de verdade. Sem essa guarda o arquivo roda de cima a baixo, e aí a
 * âncora é a primeira linha com código — pôr na linha 1 fixa deixaria a lente
 * flutuando acima de comentário de cabeçalho, longe do que ela executa.
 *
 * Sem regex, como o resto da extensão: são três testes de texto.
 */
function linhaDeEntrada(texto) {
  const linhas = texto.split('\n');
  for (let i = 0; i < linhas.length; i++) {
    const t = linhas[i].trim();
    if (t.startsWith('if') && t.includes('__name__') && t.includes('main')) return i;
  }
  for (let i = 0; i < linhas.length; i++) {
    const t = linhas[i].trim();
    if (t.length > 0 && !t.startsWith('#')) return i;
  }
  return -1;
}

const provedorLentes = {
  provideCodeLenses(doc) {
    const alvo = linhaDeEntrada(doc.getText());
    if (alvo < 0) return [];
    const faixa = new vscode.Range(alvo, 0, alvo, 0);
    return [
      new vscode.CodeLens(faixa, { title: 'Rodar',   command: 'poolscript.rodar' }),
      new vscode.CodeLens(faixa, { title: 'Depurar', command: 'poolscript.depurar' })
    ];
  }
};

/* ── depurador ──────────────────────────────────────────────────────────────
 *
 * Quem fala o Debug Adapter Protocol é o MOTOR (`pool --debug <porta>`), não
 * este arquivo: só a VM sabe onde a execução está, que frames existem e qual
 * variável mora em qual slot num dado ponto — o slot é reaproveitado entre
 * blocos, e uma cópia dessa regra aqui erraria calada. A extensão só escolhe
 * uma porta livre, sobe o processo e liga o VS Code nele.
 */

/* Uma porta livre, perguntando ao sistema em vez de chutar um número: porta
 * fixa colide quando se depura dois arquivos ao mesmo tempo. */
function portaLivre() {
  return new Promise((ok, falhou) => {
    const s = net.createServer();
    s.on('error', falhou);
    s.listen(0, '127.0.0.1', () => {
      const p = s.address().port;
      s.close(() => ok(p));
    });
  });
}

/* Espera o motor abrir a porta. O VS Code conecta assim que a fábrica retorna,
 * e o processo leva alguns milissegundos pra escutar — devolver antes disso dá
 * "connection refused" na cara do usuário, sem explicação. */
function esperaPorta(porta, prazoMs) {
  const limite = Date.now() + prazoMs;
  return new Promise((ok, falhou) => {
    const tenta = () => {
      const c = net.connect(porta, '127.0.0.1');
      c.on('connect', () => { c.destroy(); ok(); });
      c.on('error', () => {
        c.destroy();
        if (Date.now() > limite) falhou(new Error('o motor não abriu a porta ' + porta));
        else setTimeout(tenta, 50);
      });
    };
    tenta();
  });
}

const fabricaAdaptador = {
  async createDebugAdapterDescriptor(sessao) {
    const cfg = sessao.configuration;
    const programa = cfg.programa;
    if (!programa) {
      window.showErrorMessage('PoolScript: a configuração de depuração não diz qual arquivo rodar.');
      return null;
    }
    const porta = await portaLivre();
    const cwd = cfg.cwd || path.dirname(programa);
    const args = ['--debug', String(porta), programa].concat(cfg.args || []);

    if (!saidaDebug) saidaDebug = window.createOutputChannel('PoolScript (depuração)');
    saidaDebug.show(true);
    saidaDebug.appendLine('$ ' + poolBin() + ' ' + args.join(' '));

    const proc = spawn(poolBin(), args, { cwd });
    /* A saída do PROGRAMA vai pra este painel. Ela não pode ir pelo protocolo:
     * o DAP tem socket próprio justamente pra um `post()` no meio de uma
     * mensagem não quebrar o enquadramento. */
    proc.stdout.on('data', (d) => saidaDebug.append(String(d)));
    proc.stderr.on('data', (d) => saidaDebug.append(String(d)));
    proc.on('error', (e) => window.showErrorMessage('PoolScript: não consegui rodar o motor — ' + e.message));

    try {
      await esperaPorta(porta, 10000);
    } catch (e) {
      proc.kill();
      window.showErrorMessage('PoolScript: ' + e.message);
      return null;
    }
    return new vscode.DebugAdapterServer(porta);
  }
};

/* ── gráfico de execução ────────────────────────────────────────────────────
 *
 * Por onde o programa passou, e em que linha ele quebrou. O motor acumula as
 * arestas (linha → linha) durante a execução e as entrega no pedido
 * `poolscriptGrafico`; aresta repetida vira contador, então um laço de um
 * milhão de voltas é uma seta com peso, não um milhão de setas.
 */
let ultimoGrafico = null;

/* Sem regex: escapar HTML aqui é troca de literais, e é assim que o resto da
 * extensão faz. */
function escapaHtml(s) {
  return String(s)
    .split('&').join('&amp;')
    .split('<').join('&lt;')
    .split('>').join('&gt;')
    .split('"').join('&quot;');
}

function chaveNo(n) { return n.proto + ':' + n.linha; }

/* Desenha em SVG montado à mão. Nada de biblioteca de grafo: a webview do VS
 * Code não busca script de fora, e embutir uma só pra empilhar caixas custaria
 * mais do que o desenho. */
function svgDoGrafico(g) {
  const nos = g.nos || [];
  const arestas = g.arestas || [];
  if (!nos.length) return '<p>O programa não chegou a executar linha nenhuma.</p>';

  /* Ordem de execução: começa por um nó que ninguém aponta (a primeira linha) e
   * segue as arestas. Sobrando nó solto, ele entra no fim — melhor mostrar
   * fora de ordem do que sumir com ele. */
  const porChave = new Map();
  for (const n of nos) porChave.set(chaveNo(n), n);
  const apontados = new Set();
  for (const a of arestas) apontados.add(chaveNo(a.para));

  const ordem = [];
  const posto = new Set();
  const poe = (c) => {
    if (!c || posto.has(c) || !porChave.has(c)) return;
    posto.add(c); ordem.push(porChave.get(c));
    for (const a of arestas) if (chaveNo(a.de) === c) poe(chaveNo(a.para));
  };
  for (const n of nos) if (!apontados.has(chaveNo(n))) poe(chaveNo(n));
  for (const n of nos) poe(chaveNo(n));

  const ALT = 34, TOPO = 20, ESQ = 150, LARG = 300;
  const y = new Map();
  ordem.forEach((n, i) => y.set(chaveNo(n), TOPO + i * ALT));
  const altura = TOPO + ordem.length * ALT + 20;

  let s = '<svg width="100%" viewBox="0 0 ' + (ESQ + LARG + 60) + ' ' + altura
        + '" xmlns="http://www.w3.org/2000/svg">';
  s += '<defs><marker id="seta" markerWidth="8" markerHeight="8" refX="7" refY="3"'
     + ' orient="auto"><path d="M0,0 L0,6 L7,3 z" fill="currentColor"/></marker></defs>';

  /* Setas primeiro, pra ficarem ATRÁS das caixas. */
  for (const a of arestas) {
    const ya = y.get(chaveNo(a.de)), yb = y.get(chaveNo(a.para));
    if (ya === undefined || yb === undefined) continue;
    const volta = yb <= ya;                    /* aresta pra trás = laço */
    const x = ESQ - 14;
    const desvio = volta ? -34 : -14;
    s += '<path d="M' + x + ',' + (ya + 12) + ' C' + (x + desvio) + ',' + (ya + 12)
       + ' ' + (x + desvio) + ',' + (yb + 12) + ' ' + x + ',' + (yb + 12) + '"'
       + ' fill="none" stroke="currentColor" stroke-opacity="' + (volta ? 0.65 : 0.35)
       + '" marker-end="url(#seta)"/>';
    if (a.vezes > 1) {
      s += '<text x="' + (x + desvio - 4) + '" y="' + ((ya + yb) / 2 + 16)
         + '" font-size="10" text-anchor="end" fill="currentColor" opacity="0.7">'
         + a.vezes + '×</text>';
    }
  }

  for (const n of ordem) {
    const yy = y.get(chaveNo(n));
    const quebrou = n.quebrou === true;
    s += '<rect x="' + ESQ + '" y="' + yy + '" width="' + LARG + '" height="24" rx="4"'
       + ' fill="' + (quebrou ? 'var(--vscode-inputValidation-errorBackground)'
                              : 'var(--vscode-editorWidget-background)') + '"'
       + ' stroke="' + (quebrou ? 'var(--vscode-errorForeground)'
                                : 'var(--vscode-editorWidget-border)') + '"/>';
    const rotulo = (n.nome || '<module>') + '  linha ' + n.linha
                 + (quebrou ? '   ← quebrou aqui' : '');
    s += '<text x="' + (ESQ + 8) + '" y="' + (yy + 16) + '" font-size="12"'
       + ' fill="' + (quebrou ? 'var(--vscode-errorForeground)'
                              : 'var(--vscode-foreground)') + '">'
       + escapaHtml(rotulo) + '</text>';
  }
  s += '</svg>';
  return s;
}

function mostraGrafico(g) {
  const painel = window.createWebviewPanel(
    'poolscriptGrafico', 'PoolScript: gráfico de execução',
    vscode.ViewColumn.Beside, {});
  const quebrou = g.quebrou && g.quebrou.linha > 0;
  painel.webview.html =
    '<!doctype html><meta charset="utf-8">'
    + '<style>body{font-family:var(--vscode-font-family);color:var(--vscode-foreground);'
    + 'background:var(--vscode-editor-background);padding:12px}'
    + 'h2{font-size:13px;font-weight:600;margin:0 0 4px}'
    + 'p{font-size:12px;opacity:.8;margin:0 0 12px}</style>'
    + '<h2>Caminho da execução</h2>'
    + '<p>' + (quebrou
        ? 'O programa quebrou na linha ' + g.quebrou.linha + ', marcada em vermelho.'
        : 'O programa terminou sem erro.')
    + ' As setas à esquerda são os saltos; <em>N×</em> é quantas vezes o mesmo salto aconteceu.</p>'
    + svgDoGrafico(g);
}

/* Pega o gráfico ANTES de a sessão fechar: depois do `terminated` o VS Code
 * desconecta, e aí não há mais a quem perguntar. */
const fabricaRastreador = {
  createDebugAdapterTracker(sessao) {
    return {
      async onDidSendMessage(m) {
        if (!m || m.type !== 'event' || m.event !== 'terminated') return;
        try {
          const g = await sessao.customRequest('poolscriptGrafico');
          ultimoGrafico = g;
          /* Abre sozinho só quando quebrou — que é o caso em que o gráfico
           * responde a pergunta que a pessoa tem na hora. Sem erro, ela abre
           * pelo comando quando quiser. */
          if (g && g.quebrou && g.quebrou.linha > 0) mostraGrafico(g);
        } catch (e) {
          /* sessão já fechada: o comando ainda mostra o último capturado */
        }
      }
    };
  }
};

function comandoGrafico() {
  if (!ultimoGrafico) {
    window.showInformationMessage(
      'PoolScript: rode uma sessão de depuração primeiro — o gráfico é o caminho que ela percorreu.');
    return;
  }
  mostraGrafico(ultimoGrafico);
}

const provedorConfig = {
  /* Sem `launch.json`, F5 num .ps aberto tem que funcionar: o VS Code chama
   * aqui com uma configuração vazia, e é este preenchimento que evita obrigar
   * o usuário a escrever um arquivo de configuração pra depurar um arquivo. */
  resolveDebugConfiguration(pasta, cfg) {
    if (!cfg.type && !cfg.request && !cfg.name) {
      const ed = window.activeTextEditor;
      if (ed && (ed.document.languageId === 'poolscript'
              || ed.document.languageId === 'poolscript-psl')) {
        cfg.type = 'poolscript';
        cfg.name = 'Depurar o arquivo aberto';
        cfg.request = 'launch';
        cfg.programa = ed.document.uri.fsPath;
      }
    }
    if (!cfg.programa) {
      window.showWarningMessage('PoolScript: abra um .ps para depurar.');
      return undefined;
    }
    /* O motor lê o nome do PROTOCOLO (`stopOnEntry`); a configuração é escrita
     * em português. A tradução mora aqui, num lugar só. */
    if (cfg.pararNaEntrada) cfg.stopOnEntry = true;
    return cfg;
  }
};

function activate(context) {
  /* Os comandos ficam FORA do `if` do LSP: quem desliga o servidor de
   * linguagem não está pedindo pra perder o botão de rodar. */
  context.subscriptions.push(commands.registerCommand('poolscript.rodar', rodarArquivo));
  context.subscriptions.push(commands.registerCommand('poolscript.depurar', depurarArquivo));
  context.subscriptions.push(
    languages.registerCodeLensProvider(
      [{ language: 'poolscript' }, { language: 'poolscript-psl' }], provedorLentes));
  context.subscriptions.push(
    commands.registerCommand('poolscript.grafico', comandoGrafico),
    debug.registerDebugConfigurationProvider('poolscript', provedorConfig),
    debug.registerDebugAdapterDescriptorFactory('poolscript', fabricaAdaptador),
    debug.registerDebugAdapterTrackerFactory('poolscript', fabricaRastreador));

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
