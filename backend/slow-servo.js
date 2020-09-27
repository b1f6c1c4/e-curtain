const path = require('path');
const shell = require('shelljs');

module.exports = (dir) => {
  shell.exec(`${path.join(__dirname, 'slow-servo.py')} ${dir ? 'left' : 'right'}`, { silent: true }, (code, stdout, stderr) => {
    if (code) {
      console.error(`Warning: slow-servo.py died with ${code}`);
    }
    if (stderr) {
      console.error(stderr);
    }
  });
};
