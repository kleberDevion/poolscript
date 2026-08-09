// Marca não-usado (import/variável) — como o Pylance apaga o que não se usa.
const path = require('path'), Module = require('module');
const EXTDIR = path.join(__dirname, '..');
const stub = {
  CompletionItemKind: new Proxy({}, { get: (_, k) => k }),
  CompletionItem: class { constructor(l, k) { this.label = l; this.kind = k; } },
  SnippetString: class { constructor(v) { this.value = v; } },
  MarkdownString: class { constructor() {} appendCodeblock() {} appendMarkdown() {} },
  Hover: class {},
  Range: class { constructor(a, b, c, d) { this.sl = a; this.sc = b; } },
  Diagnostic: class { constructor(r, m, s) { this.range = r; this.message = m; this.severity = s; this.tags = []; } },
  DiagnosticTag: { Unnecessary: 1 },
  DiagnosticSeverity: { Hint: 3, Information: 2, Warning: 1, Error: 0 },
  languages: { registerCompletionItemProvider: () => ({ dispose() {} }), registerHoverProvider: () => ({ dispose() {} }), createDiagnosticCollection: () => ({ set() {}, delete() {}, dispose() {} }) },
  workspace: { createFileSystemWatcher: () => ({ onDidCreate() {}, onDidChange() {}, onDidDelete() {}, dispose() {} }), onDidOpenTextDocument: () => ({ dispose() {} }), onDidChangeTextDocument: () => ({ dispose() {} }), onDidCloseTextDocument: () => ({ dispose() {} }), textDocuments: [] },
};
const ol = Module._load; Module._load = function (r) { if (r === 'vscode') return stub; return ol.apply(this, arguments); };
const ext = require(path.join(EXTDIR, 'extension.js'));
const D = ext.__test__.diagnosticosNaoUsados;
function doc(t) { const L = t.split('\n'); return { getText: () => t, lineAt: (x) => ({ text: L[x] }) }; }
function nomesMarcados(t) { return D(doc(t)).map(d => d.message.match(/'([^']+)'/)[1]); }

let falhas = 0;
function checa(desc, t, deve, naoDeve) {
  const marc = new Set(nomesMarcados(t));
  const ok = (deve || []).every(x => marc.has(x)) && (naoDeve || []).every(x => !marc.has(x));
  if (!ok) { falhas++; console.log('FALHA', desc, '\n  marcados:', [...marc]); } else console.log('OK  ', desc);
}

// import não usado -> marcado; usado -> não
checa('import não usado marcado', 'from ._x import naoUso\nfrom ._y import uso\npost(uso())', ['naoUso'], ['uso']);
// variável não usada -> marcada; usada -> não
checa('variável não usada marcada', 'x = 1\ny = 2\npost(y)', ['x'], ['y']);
// var usada em expressão -> não marca
checa('var lida não marca', 'total = 10\nmetade = total / 2\npost(metade)', [], ['total', 'metade']);
// self.x não é tratado como variável solta
checa('self.x não vira "variável não usada"', 'public class C()\n{\n public self.estado: bool = none\n}', [], ['estado']);

console.log(falhas === 0 ? '\n>>> NÃO-USADO OK <<<' : `\n>>> ${falhas} FALHA(S) <<<`);
process.exit(falhas === 0 ? 0 : 1);
