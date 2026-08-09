const path = require('path');
const Mocha = require('mocha');
const fs = require('fs');
exports.run = function () {
  const mocha = new Mocha({ ui: 'tdd', color: true, timeout: 30000 });
  const dir = __dirname;
  fs.readdirSync(dir).filter(f => f.endsWith('.test.js')).forEach(f => mocha.addFile(path.join(dir, f)));
  return new Promise((resolve, reject) => {
    mocha.run(failures => failures ? reject(new Error(failures + ' falharam')) : resolve());
  });
};
