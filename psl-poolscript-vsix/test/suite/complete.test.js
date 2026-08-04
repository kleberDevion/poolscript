const assert = require('assert');
const path = require('path');
const vscode = require('vscode');

function labels(list) {
  return (list && list.items ? list.items : []).map(i =>
    typeof i.label === 'string' ? i.label : (i.label && i.label.label) || '');
}

// Abre um doc em memória com o conteúdo dado, espera a extensão analisar, e
// devolve as labels de completion na posição (linha, col).
async function completar(content, line, col) {
  const doc = await vscode.workspace.openTextDocument({ language: 'poolscript', content });
  await vscode.window.showTextDocument(doc);
  await new Promise(r => setTimeout(r, 1200));
  const list = await vscode.commands.executeCommand(
    'vscode.executeCompletionItemProvider', doc.uri, new vscode.Position(line, col));
  return labels(list);
}
function temTodos(L, nomes) { return nomes.every(n => L.includes(n)); }

suite('PoolScript — autocomplete de métodos', () => {

  test('str tipada (str x = "..."; x.) expõe métodos de string', async () => {
    const L = await completar('str nome = "Pool"\ny = nome.\n', 1, 9);
    assert.ok(temTodos(L, ['upper', 'lower', 'strip', 'split']),
      'esperava métodos de string, veio: ' + L.join(','));
  });

  test('string inferida (x = "..."; x.) expõe métodos de string', async () => {
    const L = await completar('frase = "oi mundo"\ny = frase.\n', 1, 10);
    assert.ok(temTodos(L, ['upper', 'replace', 'startswith']),
      'esperava métodos de string, veio: ' + L.join(','));
  });

  test('literal de string ("...".) expõe métodos de string', async () => {
    const L = await completar('y = "texto".\n', 0, 12);
    assert.ok(temTodos(L, ['upper', 'lower']),
      'esperava métodos de string no literal, veio: ' + L.join(','));
  });

  test('list tipada (list x = [...]; x.) expõe métodos de lista', async () => {
    const L = await completar('list nums = [1, 2, 3]\ny = nums.\n', 1, 9);
    assert.ok(temTodos(L, ['append', 'pop', 'sort']),
      'esperava métodos de lista, veio: ' + L.join(','));
  });

  test('PoolFile via loadFile (f = os.loadFile(...); f.) expõe move/copy/delete', async () => {
    const L = await completar('import os\nf = os.loadFile("img.png")\ny = f.\n', 2, 6);
    assert.ok(temTodos(L, ['move', 'copy', 'delete']),
      'esperava métodos de PoolFile, veio: ' + L.join(','));
  });

  test('PoolFile via construtor (p = PoolFile(...); p.) expõe move/copy', async () => {
    const L = await completar('p = PoolFile("a.txt")\ny = p.\n', 1, 6);
    assert.ok(temTodos(L, ['move', 'copy']),
      'esperava métodos de PoolFile, veio: ' + L.join(','));
  });
});
