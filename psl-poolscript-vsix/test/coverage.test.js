// Cobertura de LARGURA: em vez de casos escolhidos a dedo, varre TODO o
// metadata e prova que o completion expõe cada módulo, cada membro, cada
// argumento nomeado e cada cadeia (mod.fn() -> Classe -> métodos). Se algum
// tipo/lib parar de aparecer, este teste quebra.
const path = require('path');
const Module = require('module');
const EXTDIR = path.join(__dirname, '..');

const stub = {
  CompletionItemKind: new Proxy({}, { get: (_, k) => k }),
  CompletionItem: class { constructor(l, k) { this.label = l; this.kind = k; } },
  SnippetString: class { constructor(v) { this.value = v; } },
  MarkdownString: class { constructor(v) { this.value = v || ''; } appendCodeblock(c) { this.value += c; } appendMarkdown(m) { this.value += m; } },
  Hover: class {}, Range: class {},
  languages: { registerCompletionItemProvider: (s, p) => { stub._c = p; return { dispose() {} }; }, registerHoverProvider: () => ({ dispose() {} }) },
  workspace: { createFileSystemWatcher: () => ({ onDidCreate() {}, onDidChange() {}, onDidDelete() {}, dispose() {} }) },
};
const ol = Module._load;
Module._load = function (r) { if (r === 'vscode') return stub; return ol.apply(this, arguments); };

const ext = require(path.join(EXTDIR, 'extension.js'));
ext.activate({ extensionPath: EXTDIR, subscriptions: [] });
const P = stub._c, META = ext.__test__.getMeta();

let _id = 0;
function comp(text, line) {
  const L = text.split('\n'), uri = 'f' + (_id++);
  const d = { uri: { toString: () => uri, fsPath: uri }, version: 1, getText: () => text, lineAt: (l) => ({ text: L[l] }) };
  const items = P.provideCompletionItems(d, { line, character: L[line].length });
  return new Set((items || []).map(i => (typeof i.label === 'string' ? i.label : i.label.label)));
}

let modBad = 0, memTot = 0, memOK = 0, argFns = 0, argOK = 0, clsOK = 0, clsBad = 0;
const modN = Object.keys(META.modules).length;

for (const [mod, info] of Object.entries(META.modules)) {
  const got = comp(mod + '.', 0);
  const membros = info.members.map(m => m.name);
  const faltou = membros.filter(n => !got.has(n));
  memTot += membros.length; memOK += membros.length - faltou.length;
  if (faltou.length) { modBad++; console.log('FALHA módulo', mod, 'faltou:', faltou); }
  for (const m of info.members) {
    if (m.kind === 'function' && m.params && m.params.length) {
      argFns++;
      const g = comp('x = ' + mod + '.' + m.name + '(', 0);
      const fa = m.params.filter(p => !g.has(p.name + '='));
      if (!fa.length) argOK++; else console.log('FALHA arg', mod + '.' + m.name, fa.map(p => p.name));
    }
  }
}
for (const [mod, info] of Object.entries(META.modules)) {
  for (const m of info.members) {
    if (m.kind === 'function' && m.returns && META.classes[m.returns]) {
      const got = comp('x = ' + mod + '.' + m.name + '()\nx.', 1);
      const membros = META.classes[m.returns].members.map(x => x.name);
      const faltou = membros.filter(n => !got.has(n));
      if (!faltou.length) clsOK++; else { clsBad++; console.log('FALHA classe', m.returns, 'via', mod + '.' + m.name, faltou); }
    }
  }
}

console.log(`\nmódulos: ${modN - modBad}/${modN} | membros: ${memOK}/${memTot} | args nomeados: ${argOK}/${argFns} | cadeias->classe: ${clsOK}/${clsOK + clsBad}`);
const falhou = modBad || (memOK !== memTot) || (argOK !== argFns) || clsBad;
console.log(falhou ? '>>> COBERTURA INCOMPLETA <<<' : '>>> COBERTURA 100% <<<');
process.exit(falhou ? 1 : 0);
