const path = require('path');
const axios = require('axios').default;
const shell = require('shelljs');
const EventEmitter = require('events');

EventEmitter.defaultMaxListeners = 30;

const interval = 500;

function measure(endpoint) {
  shell.exec(path.join(__dirname, 'rf.py'), { silent: true }, (code, stdout, stderr) => {
    if (code) {
      console.error(`Warning: rf.py died with ${code}`);
    }
    if (stderr) {
      console.error(stderr);
    }
    const v = +stdout;
    axios({
      method: 'post',
      url: `http://${endpoint}/rf`,
      data: { v },
      validateStatus: null,
    });
    setTimeout(measure, interval, endpoint);
  });
}

module.exports = measure;
