const path = require('path');
const axios = require('axios').default;
const shell = require('shelljs');

const interval = 200;

function measure(endpoint) {
  shell.exec(path.join(__dirname, 'rf.py'), (code, stdout, stderr) => {
    if (code) {
      console.error(`Warning: rf.py died with ${code}`);
    }
    if (stderr) {
      console.error(stderr);
    }
    const v = +stdout;
    if (v) {
      axios({
        method: 'post',
        url: `http://${host}/rf`,
        data: {
          a: v & 8,
          b: v & 4,
          c: v & 2,
          d: v & 1,
        },
      });
    }
  });
}

module.exports = (ep) => { setInterval(measure, interval, ep); };
