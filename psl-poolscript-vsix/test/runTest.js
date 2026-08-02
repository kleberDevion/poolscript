const path = require('path');
// Se o ambiente tiver ELECTRON_RUN_AS_NODE=1 (comum dentro de editores/CI que
// são Electron), o binário do VSCode roda como Node e REJEITA todos os args
// (--extensionDevelopmentPath vira "bad option"). Tira antes de subir o VSCode.
delete process.env.ELECTRON_RUN_AS_NODE;
const { runTests } = require('@vscode/test-electron');
async function main() {
  try {
    const extensionDevelopmentPath = path.resolve(__dirname, '..');
    const extensionTestsPath = path.resolve(__dirname, './suite/index');
    await runTests({
      extensionDevelopmentPath, extensionTestsPath,
      launchArgs: ['--skip-welcome', '--skip-release-notes'],
    });
  } catch (err) { console.error('HARNESS FAIL:', err); process.exit(1); }
}
main();
