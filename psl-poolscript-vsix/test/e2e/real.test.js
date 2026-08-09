// Testa a extensão no VS CODE DE VERDADE (Extension Host), agora em modo LSP:
// o runTest.js aponta POOLSCRIPT_LSP_CMD pro servidor real do repositório, então
// completion/hover/diagnóstico aqui atravessam o protocolo inteiro
// (editor -> vscode-languageclient -> poolscript.lsp.server -> parser real).
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
// o servidor Python demora ~2s pra subir na 1ª vez — repete até responder
async function completarAte(doc, linha, col, precisa, ms) {
  const fim = Date.now() + (ms || 15000);
  let nomes = [];
  while (Date.now() < fim) {
    nomes = await completar(doc, linha, col);
    if (nomes.includes(precisa)) return nomes;
    await new Promise(r => setTimeout(r, 500));
  }
  return nomes;
}
async function pairar(doc, linha, col) {
  const r = await vscode.commands.executeCommand('vscode.executeHoverProvider', doc.uri, new vscode.Position(linha, col));
  const out = [];
  for (const h of (r || [])) for (const c of h.contents) out.push(typeof c === 'string' ? c : c.value);
  return out.join('\n');
}

suite('PoolScript no VS Code REAL (modo LSP)', () => {
  test('a extensão ativa e reconhece poolscript', async () => {
    const doc = await abre('a.ps', 'post("oi")\n');
    assert.strictEqual(doc.languageId, 'poolscript');
    await new Promise(r => setTimeout(r, 1500));  // dá tempo do onLanguage + LSP
  });

  test('completion: cur. -> métodos de DbCursor (cadeia type-aware)', async () => {
    const doc = await abre('b.ps', 'import psodbc\nconn = psodbc.connect(base="d")\ncur = conn.cursor()\ncur.\n');
    const n = await completarAte(doc, 3, 4, 'fetchall');
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
    const n = await completarAte(doc, 0, 'from ._conn import '.length, 'conectar');
    assert.ok(n.includes('conectar'), `faltou conectar; veio [${n.slice(0, 20)}]`);
    assert.ok(!n.includes('post'), `vazou builtin post; veio [${n.slice(0, 20)}]`);
  });

  test('hover: keyword catch é rico', async () => {
    const doc = await abre('c.ps', 'try { raise "x" } catch (e) { post(e) }\n');
    let t = '';
    const fim = Date.now() + 15000;
    while (Date.now() < fim) {
      t = await pairar(doc, 0, 20);
      if (/catch/.test(t)) break;
      await new Promise(r => setTimeout(r, 500));
    }
    assert.ok(/catch/.test(t), `hover vazio/errado: ${JSON.stringify(t)}`);
  });

  test('diagnóstico: import não usado chega do servidor (apagado)', async () => {
    const doc = await abre('d.ps', 'import os\npost("oi")\n');
    let diags = [];
    const fim = Date.now() + 15000;
    while (Date.now() < fim) {
      diags = vscode.languages.getDiagnostics(doc.uri) || [];
      if (diags.some(d => /não é usado/.test(d.message))) break;
      await new Promise(r => setTimeout(r, 500));
    }
    const alvo = diags.find(d => /não é usado/.test(d.message));
    assert.ok(alvo, `sem diagnóstico de não-usado: ${JSON.stringify(diags.map(d => d.message))}`);
    assert.ok((alvo.tags || []).includes(vscode.DiagnosticTag.Unnecessary), 'sem tag Unnecessary');
  });

  test('diagnóstico: erro de sintaxe do parser REAL', async () => {
    const doc = await abre('e.ps', 'action f( {\n');
    let diags = [];
    const fim = Date.now() + 15000;
    while (Date.now() < fim) {
      diags = (vscode.languages.getDiagnostics(doc.uri) || [])
        .filter(d => d.severity === vscode.DiagnosticSeverity.Error);
      if (diags.length) break;
      await new Promise(r => setTimeout(r, 500));
    }
    assert.ok(diags.length, 'erro de sintaxe não virou diagnóstico');
  });
});
