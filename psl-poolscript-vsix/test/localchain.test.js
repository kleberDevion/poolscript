// Prova a cadeia por IMPORT LOCAL — o caso real do usuário:
//   _connect.ps:  action connection() { return psodbc.connect(...) }
//   _user.ps:     from ._connect import connection
//                 conn = connection()   -> DbConnection
//                 cur  = conn.cursor()  -> DbCursor
//                 cur.<TAB>             -> fetchall/fetchone/...
const assert = require('assert');
const fs = require('fs');
const os = require('os');
const path = require('path');
const Module = require('module');
const EXTDIR = path.join(__dirname, '..');

const stub = {
  CompletionItemKind: new Proxy({}, { get: (_, k) => k }),
  CompletionItem: class { constructor(l, k) { this.label = l; this.kind = k; } },
  SnippetString: class { constructor(v) { this.value = v; } },
  MarkdownString: class { constructor() { this.value = ''; } appendCodeblock(c) { this.value += c; } appendMarkdown(m) { this.value += m; } },
  Hover: class {}, Range: class {},
  languages: { registerCompletionItemProvider: (s, p) => { stub._c = p; return { dispose() {} }; }, registerHoverProvider: () => ({ dispose() {} }) },
  workspace: { createFileSystemWatcher: () => ({ onDidCreate() {}, onDidChange() {}, onDidDelete() {}, dispose() {} }) },
};
const ol = Module._load;
Module._load = function (r) { if (r === 'vscode') return stub; return ol.apply(this, arguments); };
const ext = require(path.join(EXTDIR, 'extension.js'));
ext.activate({ extensionPath: EXTDIR, subscriptions: [] });
const P = stub._c;

const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'pslocal-'));
fs.writeFileSync(path.join(dir, '_connect.ps'),
  'import psodbc\naction connection() {\n    return psodbc.connect(driver="postgres", base="db")\n}\n');

function comp(fsPath, text, line, col) {
  const L = text.split('\n');
  const doc = { uri: { toString: () => fsPath, fsPath }, version: 1, getText: () => text, lineAt: (l) => ({ text: L[l] }) };
  const items = P.provideCompletionItems(doc, { line, character: col });
  return (items || []).map(i => (typeof i.label === 'string' ? i.label : i.label.label));
}

let falhas = 0;
function checa(desc, got, deve) {
  const g = new Set(got);
  const ok = deve.every(x => g.has(x));
  if (!ok) { falhas++; console.log('FALHA', desc, '\n  faltou:', deve.filter(x => !g.has(x)), '\n  got:', got.slice(0, 12)); }
  else console.log('OK  ', desc);
}

const user = 'from ._connect import connection\nconn = connection()\ncur = conn.cursor()\ncur.';
checa('import local: connection() -> DbConnection -> cursor()', comp(path.join(dir, '_user.ps'), user, 3, 4),
  ['fetchall', 'fetchone', 'fetchmany', 'rowcount', 'execute']);

// conn. direto também
const user2 = 'from ._connect import connection\nconn = connection()\nconn.';
checa('import local: conn. -> DbConnection (cursor/commit/close)', comp(path.join(dir, '_user.ps'), user2, 2, 5),
  ['cursor', 'commit', 'close']);

console.log(falhas === 0 ? '\n>>> IMPORT LOCAL RESOLVE A CADEIA <<<' : `\n>>> ${falhas} FALHA(S) <<<`);
process.exit(falhas === 0 ? 0 : 1);
