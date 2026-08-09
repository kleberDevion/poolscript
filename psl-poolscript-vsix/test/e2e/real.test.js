// Testa a extensão no VS CODE DE VERDADE (Extension Host) — os comandos nativos
// do editor, não stub. Pega erro de ativação/registro que o stub não pega.
const assert = require('assert');
const vscode = require('vscode');
const fs = require('fs');
const os = require('os');
const path = require('path');

async function abre(nome, conteudo) {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'e2e-'));
  const arq = path.join(dir, nome);
  fs.writeFileSync(arq, conteudo);
  const doc = await vscode.workspace.openTextDocument(arq);
  await vscode.window.showTextDocument(doc);
  return doc;
}
async function completar(doc, linha, col) {
  const r = await vscode.commands.executeCommand('vscode.executeCompletionItemProvider', doc.uri, new vscode.Position(linha, col));
  return (r ? r.items : []).map(i => (typeof i.label === 'string' ? i.label : i.label.label));
}
async function pairar(doc, linha, col) {
  const r = await vscode.commands.executeCommand('vscode.executeHoverProvider', doc.uri, new vscode.Position(linha, col));
  const out = [];
  for (const h of (r || [])) for (const c of h.contents) out.push(typeof c === 'string' ? c : c.value);
  return out.join('\n');
}

suite('PoolScript no VS Code REAL', () => {
  test('a extensão ativa e reconhece poolscript', async () => {
    const doc = await abre('a.ps', 'post("oi")\n');
    assert.strictEqual(doc.languageId, 'poolscript');
    await new Promise(r => setTimeout(r, 800));  // dá tempo do onLanguage ativar
  });

  test('completion: cur. -> métodos de DbCursor', async () => {
    const doc = await abre('b.ps', 'import psodbc\nconn = psodbc.connect(base="d")\ncur = conn.cursor()\ncur.\n');
    const n = await completar(doc, 3, 4);
    for (const m of ['fetchall', 'fetchone', 'rowcount'])
      assert.ok(n.includes(m), `faltou ${m}; veio [${n.slice(0, 20)}]`);
  });

  test('completion: from ._x import <TAB> -> exports do arquivo, sem builtins', async () => {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'e2e-'));
    fs.writeFileSync(path.join(dir, '_conn.ps'), 'action conectar() { return 1 }\n');
    const arq = path.join(dir, 'main.ps');
    fs.writeFileSync(arq, 'from ._conn import ');
    const doc = await vscode.workspace.openTextDocument(arq);
    await vscode.window.showTextDocument(doc);
    const n = await completar(doc, 0, 'from ._conn import '.length);
    assert.ok(n.includes('conectar'), `faltou conectar; veio [${n.slice(0, 20)}]`);
    assert.ok(!n.includes('post'), `vazou builtin post; veio [${n.slice(0, 20)}]`);
  });

  test('hover: keyword catch é rico', async () => {
    const doc = await abre('c.ps', 'try { raise "x" } catch (e) { post(e) }\n');
    const t = await pairar(doc, 0, 20);
    assert.ok(/catch/.test(t), `hover vazio/errado: ${JSON.stringify(t)}`);
  });
});
