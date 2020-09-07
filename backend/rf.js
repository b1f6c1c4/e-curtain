const path = require('path');
const axios = require('axios').default;
const shell = require('shelljs');

const interval = 200;

let vo = 0;
function measure(endpoint) {
  shell.exec(path.join(__dirname, 'rf.py'), (code, stdout, stderr) => {
    if (code) {
      console.error(`Warning: rf.py died with ${code}`);
    }
    if (stderr) {
      console.error(stderr);
    }
    const v = +stdout;
    if (v && v != vo) {
      axios({
        method: 'post',
        url: `http://${host}/rf`,
        data: { v },
      });
    }
    vo = v;
  });
}

module.exports = (ep) => { setInterval(measure, interval, ep); };
