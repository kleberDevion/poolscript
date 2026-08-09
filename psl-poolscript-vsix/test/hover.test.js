// Prova que o hover é RICO (assinatura em code block + descrição + exemplo),
// não "NOME — palavra-chave" em texto cinza.
const path = require('path');
const Module = require('module');
const EXTDIR = path.join(__dirname, '..');

let HOV = null;
const stub = {
  CompletionItemKind: new Proxy({}, { get: (_, k) => k }),
  CompletionItem: class { constructor(l, k) { this.label = l; this.kind = k; } },
  SnippetString: class { constructor(v) { this.value = v; } },
  MarkdownString: class { constructor() { this.value = ''; } appendCodeblock(c, l) { this.value += '```' + (l || '') + '\n' + c + '\n```\n'; } appendMarkdown(m) { this.value += m; } },
  Hover: class { constructor(c) { this.contents = c; } },
  Range: class {},
  languages: { registerCompletionItemProvider: () => ({ dispose() {} }), registerHoverProvider: (s, p) => { HOV = p; return { dispose() {} }; } },
  workspace: { createFileSystemWatcher: () => ({ onDidCreate() {}, onDidChange() {}, onDidDelete() {}, dispose() {} }) },
};
const ol = Module._load;
Module._load = function (r) { if (r === 'vscode') return stub; return ol.apply(this, arguments); };
const ext = require(path.join(EXTDIR, 'extension.js'));
ext.activate({ extensionPath: EXTDIR, subscriptions: [] });

let _id = 0;
function hover(text, line, ch) {
  const L = text.split('\n');
  const d = {
    uri: { toString: () => 'h' + (_id++), fsPath: '/h.ps' }, version: 1,
    getText: (r) => (r && r._w !== undefined ? r._w : text),
    lineAt: (l) => ({ text: L[l] }),
    getWordRangeAtPosition: (pos) => {
      const m = [...L[pos.line].matchAll(/[A-Za-z_]\w*/g)].find(mm => mm.index <= pos.character && pos.character <= mm.index + mm[0].length);
      return m ? { start: { line: pos.line, character: m.index }, end: { line: pos.line, character: m.index + m[0].length }, _w: m[0] } : null;
    },
  };
  const h = HOV.provideHover(d, { line, character: ch });
  return h ? h.contents.value : null;
}

let falhas = 0;
function checa(desc, txt, deve) {
  const ok = txt && deve.every(s => txt.includes(s));
  if (!ok) { falhas++; console.log('FALHA', desc, '\n  got:', JSON.stringify(txt)); }
  else console.log('OK  ', desc);
}

// keyword: code block + resumo + exemplo (não "é palavra-chave" pelado)
checa('hover catch = rico (sig+resumo+exemplo)',
  hover('try { raise "x" } catch (e) { post(e) }', 0, 20),
  ['```poolscript', 'catch (e)', 'Captura o erro', '**Exemplo**']);
// builtin
checa('hover post = builtin rico', hover('post("oi")', 0, 1),
  ['```poolscript', 'post(', 'Imprime']);
// módulo
checa('hover date = módulo', hover('import date\ndate', 1, 1),
  ['```poolscript', 'date', 'módulo']);
// membro de tipo (cadeia): fetchall com returns
checa('hover cur.fetchall = método com -> list',
  hover('import psodbc\nconn = psodbc.connect(base="d")\ncur = conn.cursor()\ncur.fetchall', 3, 8),
  ['```poolscript', 'fetchall', '-> list']);

console.log(falhas === 0 ? '\n>>> HOVER RICO OK <<<' : `\n>>> ${falhas} FALHA(S) <<<`);
process.exit(falhas === 0 ? 0 : 1);
