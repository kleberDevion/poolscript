const assert = require('assert');
const path = require('path');
const vscode = require('vscode');

function labels(list) {
  return (list && list.items ? list.items : []).map(i =>
    typeof i.label === 'string' ? i.label : (i.label && i.label.label) || '');
}
async function abre() {
  const uri = vscode.Uri.file(path.resolve(__dirname, '../fixtures/sample.ps'));
  const doc = await vscode.workspace.openTextDocument(uri);
  await vscode.window.showTextDocument(doc);
  await new Promise(r => setTimeout(r, 3000)); // deixa a extensão ativar/analisar
  return doc;
}
function acha(doc, trecho, offsetExtra = 0) {
  for (let l = 0; l < doc.lineCount; l++) {
    const idx = doc.lineAt(l).text.indexOf(trecho);
    if (idx >= 0) return new vscode.Position(l, idx + trecho.length + offsetExtra);
  }
  throw new Error('trecho nao achado: ' + trecho);
}

suite('PoolScript — autocomplete', () => {
  let doc;
  suiteSetup(async () => { doc = await abre(); });

  test('f. (PoolFile) sugere move/copy/delete', async () => {
    const pos = acha(doc, 'f.');
    const list = await vscode.commands.executeCommand('vscode.executeCompletionItemProvider', doc.uri, pos);
    const L = labels(list);
    console.log('[[ f. ]]', JSON.stringify(L));
    assert.ok(L.includes('move') && L.includes('copy') && L.includes('delete'),
      'PoolFile deveria sugerir move/copy/delete, veio: ' + L.join(','));
  });

  test('cors. sugere options/origins', async () => {
    const doc2 = await vscode.workspace.openTextDocument({ language: 'poolscript',
      content: 'import jinker\ncors = jinker.cors\nx = cors.\n' });
    await vscode.window.showTextDocument(doc2);
    await new Promise(r => setTimeout(r, 1500));
    const pos = new vscode.Position(2, 9);
    const list = await vscode.commands.executeCommand('vscode.executeCompletionItemProvider', doc2.uri, pos);
    const L = labels(list);
    console.log('[[ cors. ]]', JSON.stringify(L));
    assert.ok(L.includes('options') && L.includes('origins'),
      'cors deveria sugerir options/origins, veio: ' + L.join(','));
  });

  test('.route(...) mostra signature com methods/auth/middleware', async () => {
    const pos = acha(doc, '@app.route("/x", ');
    const sig = await vscode.commands.executeCommand('vscode.executeSignatureHelpProvider', doc.uri, pos);
    const txt = sig && sig.signatures ? sig.signatures.map(s => s.label).join(' | ') : '(nada)';
    console.log('[[ route sig ]]', txt);
    assert.ok(/methods/.test(txt) && /auth/.test(txt) && /middleware/.test(txt),
      'route deveria expor methods/auth/middleware, veio: ' + txt);
  });
});
