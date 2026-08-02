const path = require('path');
const Mocha = require('mocha');
const fs = require('fs');
function run() {
  const mocha = new Mocha({ ui: 'tdd', color: true, timeout: 60000 });
  const dir = __dirname;
  fs.readdirSync(dir).filter(f => f.endsWith('.test.js')).forEach(f => mocha.addFile(path.join(dir, f)));
  return new Promise((resolve, reject) => {
    try { mocha.run(failures => failures ? reject(new Error(failures + ' testes falharam')) : resolve()); }
    catch (e) { reject(e); }
  });
}
module.exports = { run };
