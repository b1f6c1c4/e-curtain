const path = require('path');
const axios = require('axios').default;
const shell = require('shelljs');
const EventEmitter = require('events');

EventEmitter.defaultMaxListeners = 30;

const interval = 100;

let vo = 0;
function measure(endpoint) {
  shell.exec(path.join(__dirname, 'rf.py'), { silent: true }, (code, stdout, stderr) => {
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
        url: `http://${endpoint}/rf`,
        data: { v },
      });
    }
    vo = v;
  });
}

module.exports = (ep) => { setInterval(measure, interval, ep); };
