const path = require('path');
const { runTests } = require('@vscode/test-electron');
(async () => {
  try {
    await runTests({
      extensionDevelopmentPath: path.resolve(__dirname, '..'),
      extensionTestsPath: path.resolve(__dirname, './e2e/index.js'),
    });
  } catch (e) {
    console.error('FALHA runTest:', e);
    process.exit(1);
  }
})();
