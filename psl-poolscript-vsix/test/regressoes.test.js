// Regressões que a reescrita "do zero" tinha perdido e voltaram:
//  - self. expõe campos public/private + métodos da classe
//  - os TRÊS request: jinker (proxy) vs cliente HTTP vs requests
//  - run_selfwith_ sugerido
const path = require('path'), Module = require('module');
const EXTDIR = path.join(__dirname, '..');
const stub = {
  CompletionItemKind: new Proxy({}, { get: (_, k) => k }),
  CompletionItem: class { constructor(l, k) { this.label = l; this.kind = k; } },
  SnippetString: class { constructor(v) { this.value = v; } },
  MarkdownString: class { constructor() { this.value = ''; } appendCodeblock() {} appendMarkdown() {} },
  Hover: class {}, Range: class {},
  languages: { registerCompletionItemProvider: (s, p) => { stub._c = p; return { dispose() {} }; }, registerHoverProvider: () => ({ dispose() {} }) },
  workspace: { createFileSystemWatcher: () => ({ onDidCreate() {}, onDidChange() {}, onDidDelete() {}, dispose() {} }) },
};
const ol = Module._load; Module._load = function (r) { if (r === 'vscode') return stub; return ol.apply(this, arguments); };
const ext = require(path.join(EXTDIR, 'extension.js'));
ext.activate({ extensionPath: EXTDIR, subscriptions: [] });
const P = stub._c; let id = 0;
function comp(t, l, c) { const L = t.split('\n'); const d = { uri: { toString: () => 'u' + (id++), fsPath: '/f.ps' }, version: 1, getText: () => t, lineAt: x => ({ text: L[x] }) }; return (P.provideCompletionItems(d, { line: l, character: c }) || []).map(i => (typeof i.label === 'string' ? i.label : i.label.label)); }
let falhas = 0;
function checa(desc, got, deve, naoDeve) { const g = new Set(got); const ok = (deve || []).every(x => g.has(x)) && (naoDeve || []).every(x => !g.has(x)); if (!ok) { falhas++; console.log('FALHA', desc, '\n  got:', got.slice(0, 12)); } else console.log('OK  ', desc); }

checa('self. -> campos public/private + métodos',
  comp('public class Janela()\n{\n    public self.estadoFechar: bool = none;\n    public int reaction closeButton(self,)\n    {\n        self.\n    }\n}', 5, 13),
  ['estadoFechar', 'closeButton']);
checa('jinker request. -> proxy (method/path/header)',
  comp('from jinker import Jinker\nrequest.', 1, 8), ['method', 'path', 'header', 'get_json', 'path_param', 'file']);
checa('cliente request. -> get/post (sem method)',
  comp('import request\nrequest.', 1, 8), ['get', 'post', 'ws_connect'], ['method']);
checa('requests. (com s) -> cliente',
  comp('import requests\nrequests.', 1, 9), ['post', 'get']);
checa('run_selfwith_ sugerido no topo',
  comp('x = 1\n', 1, 0), ['run_selfwith_']);

console.log(falhas === 0 ? '\n>>> REGRESSÕES OK <<<' : `\n>>> ${falhas} FALHA(S) <<<`);
process.exit(falhas === 0 ? 0 : 1);
