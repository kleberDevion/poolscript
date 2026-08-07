const path = require('path');
const fs = require('fs');
const os = require('os');
// Se o ambiente tiver ELECTRON_RUN_AS_NODE=1 (comum dentro de editores/CI que
// são Electron), o binário do VSCode roda como Node e REJEITA todos os args
// (--extensionDevelopmentPath vira "bad option"). Tira antes de subir o VSCode.
delete process.env.ELECTRON_RUN_AS_NODE;
const { runTests } = require('@vscode/test-electron');

// POOLSCRIPT_HOME temporário com uma lib "instalada", pra testar o
// reconhecimento de libs de ~/.poolscript/libs (indexInstalledLibs). Precisa
// existir ANTES da extensão ativar (a indexação roda na activação).
function preparaLibsInstaladas() {
  const home = fs.mkdtempSync(path.join(os.tmpdir(), 'pshome-'));
  const libs = path.join(home, 'libs');
  fs.mkdirSync(libs, { recursive: true });
  fs.writeFileSync(path.join(libs, 'greetlib.ps'),
    'action saudar(nome) { return "Oi " nome }\n'
    + 'Entity Pessoa() { action __init__(self, n) { self.n = n } }\n', 'utf-8');
  return home;
}

async function main() {
  try {
    const extensionDevelopmentPath = path.resolve(__dirname, '..');
    const extensionTestsPath = path.resolve(__dirname, './suite/index');
    const poolHome = preparaLibsInstaladas();
    await runTests({
      extensionDevelopmentPath, extensionTestsPath,
      launchArgs: ['--skip-welcome', '--skip-release-notes'],
      extensionTestsEnv: { POOLSCRIPT_HOME: poolHome },
    });
  } catch (err) { console.error('HARNESS FAIL:', err); process.exit(1); }
}
main();
