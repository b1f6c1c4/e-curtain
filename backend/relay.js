const path = require('path');
const shell = require('shelljs');

module.exports = (on) => {
  shell.exec(`${path.join(__dirname, 'relay.py')} ${+on}`, { silent: true }, (code, stdout, stderr) => {
    if (code) {
      console.error(`Warning: relay.py died with ${code}`);
    }
    if (stderr) {
      console.error(stderr);
    }
  });
};
