const path = require('path');
const { runTests } = require('@vscode/test-electron');
(async () => {
  try {
    const raiz = path.resolve(__dirname, '..', '..');
    await runTests({
      extensionDevelopmentPath: path.resolve(__dirname, '..'),
      extensionTestsPath: path.resolve(__dirname, './e2e/index.js'),
      // roda o Extension Host com o LSP REAL do repositório — a extensão vira
      // cliente do poolscript.lsp.server, arquitetura Pylance de verdade
      extensionTestsEnv: {
        POOLSCRIPT_LSP_CMD: JSON.stringify(['python3', '-m', 'poolscript.lsp.server']),
        PYTHONPATH: path.join(raiz, 'src'),
      },
    });
  } catch (e) {
    console.error('FALHA runTest:', e);
    process.exit(1);
  }
})();
