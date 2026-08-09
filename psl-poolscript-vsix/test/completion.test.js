// Harness: stuba o `vscode`, carrega o extension.js REAL e dirige o completion.
const path = require('path');
const Module = require('module');
const EXTDIR = require('path').join(__dirname, '..');

let capturedCompletion = null;
const vscodeStub = {
  CompletionItemKind: new Proxy({}, { get: (_, k) => k }),
  CompletionItem: class { constructor(label, kind) { this.label = label; this.kind = kind; } },
  SnippetString: class { constructor(v) { this.value = v; } },
  MarkdownString: class { constructor(v) { this.value = v || ''; } appendCodeblock(c) { this.value += c; } appendMarkdown(m) { this.value += m; } },
  Hover: class { constructor(c) { this.contents = c; } },
  Range: class {},
  languages: {
    registerCompletionItemProvider: (sel, prov) => { capturedCompletion = prov; return { dispose() {} }; },
    registerHoverProvider: () => ({ dispose() {} }),
  },
  workspace: { createFileSystemWatcher: () => ({ onDidCreate() {}, onDidChange() {}, onDidDelete() {}, dispose() {} }) },
};
const origLoad = Module._load;
Module._load = function (request) { if (request === 'vscode') return vscodeStub; return origLoad.apply(this, arguments); };

const ext = require(path.join(EXTDIR, 'extension.js'));
ext.activate({ extensionPath: EXTDIR, subscriptions: [] });
const P = capturedCompletion;

let _id = 0;
function fakeDoc(text) {
  const linhas = text.split('\n');
  const uri = 'file:///fake' + (_id++) + '.ps';   // uri ÚNICA por doc (evita colisão de cache)
  return { uri: { toString: () => uri, fsPath: uri }, version: 1, getText: () => text, lineAt: (l) => ({ text: linhas[l] }) };
}
// posição no fim de uma linha específica
function fimDaLinha(text, l) { return { line: l, character: text.split('\n')[l].length }; }

function nomes(items) { return (items || []).map(i => (typeof i.label === 'string' ? i.label : i.label.label)); }
let falhas = 0;
function checa(desc, got, deve, naoDeve) {
  const g = new Set(got);
  const okTem = (deve || []).every(x => g.has(x));
  const okNao = (naoDeve || []).every(x => !g.has(x));
  const ok = okTem && okNao;
  if (!ok) falhas++;
  console.log(`${ok ? 'OK  ' : 'FALHA'} ${desc}`);
  if (!ok) {
    if (!okTem) console.log('   faltou:', (deve || []).filter(x => !g.has(x)));
    if (!okNao) console.log('   vazou :', (naoDeve || []).filter(x => g.has(x)));
    console.log('   got   :', got.slice(0, 30));
  }
}
function comp(text, line) { const d = fakeDoc(text); return nomes(P.provideCompletionItems(d, fimDaLinha(text, line))); }

// 1. cadeia DB: connect() -> DbConnection; conn.cursor() -> DbCursor
const src1 = 'import psodbc\nconn = psodbc.connect(driver="sqlite", base="x.db")\ncur = conn.cursor()\ncur.';
checa('cur. -> métodos de DbCursor', comp(src1, 3),
  ['fetchall', 'fetchone', 'fetchmany', 'rowcount', 'execute', 'close']);

// 2. conn. -> DbConnection
checa('conn. -> cursor/commit/close', comp('import psodbc\nconn = psodbc.connect(base="x.db")\nconn.', 2),
  ['cursor', 'commit', 'close'], ['fetchall']);

// 3. sem falso positivo: string não vaza .cursor
checa('string NÃO vaza .cursor', comp('nome = "kleber"\nnome.', 1),
  ['upper', 'lower'], ['cursor', 'fetchall', 'commit']);

// 4. tipo desconhecido -> nada
checa('tipo desconhecido -> nenhuma sugestão', comp('x = coisa_desconhecida()\nx.', 1), []);
{ const g = comp('x = coisa_desconhecida()\nx.', 1); if (g.length) { falhas++; console.log('   FALHA: deveria ser vazio, veio', g); } }

// 5. argumento nomeado: connect( -> driver=/host=/port=/base=
checa('connect( -> args nomeados', comp('import psodbc\nc = psodbc.connect(', 1),
  ['driver=', 'host=', 'port=', 'base=']);

// 6. connect(driver="mongo") -> MongoConnection
checa('connect(driver="mongo") -> MongoConnection', comp('import psodbc\nm = psodbc.connect(driver="mongo", base="db")\nm.', 2),
  ['collection', 'close'], ['cursor', 'fetchall']);

// 7. str tipada -> métodos de string
checa('str s -> métodos de string, sem .cursor', comp('str s = "oi"\ns.', 1),
  ['upper', 'lower', 'split'], ['cursor']);

// 8. cadeia mais longa: cur.execute()... (execute retorna None -> nada depois)
checa('conn.cursor(). -> DbCursor direto na cadeia', comp('import psodbc\nconn = psodbc.connect(base="d")\nconn.cursor().', 2),
  ['fetchall', 'rowcount']);

// 9. date.hora( -> hours=/minutes=/days=
checa('date.hora( -> hours=/minutes=/days=', comp('import date\nd = date.hora(', 1),
  ['hours=', 'minutes=', 'days=']);

console.log(falhas === 0 ? '\n>>> TODOS OS CENÁRIOS PASSARAM <<<' : `\n>>> ${falhas} FALHA(S) <<<`);
process.exit(falhas === 0 ? 0 : 1);
