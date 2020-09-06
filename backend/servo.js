const path = require('path');
const shell = require('shelljs');

module.exports = (port, angle) => {
  shell.exec(`${path.join(__dirname, 'servo.py')} ${port} ${angle}`, (code, stdout, stderr) => {
    if (code) {
      console.error(`Warning: servo.py died with ${code}`);
    }
    if (stderr) {
      console.error(stderr);
    }
  });
};
