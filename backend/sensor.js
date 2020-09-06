const path = require('path');
const axios = require('axios').default;
const shell = require('shelljs');

const interval = 10e3;

function measure(location, endpoint) {
  shell.exec(path.join(__dirname, 'sensor.py'), (code, stdout, stderr) => {
    if (code) {
      console.error(`Warning: sensor.py died with ${code}`);
    }
    if (stderr) {
      console.error(stderr);
    }
    const [rh, t] = stdout.split('\n').map((s) => +s);
    axios({
      method: 'post',
      url: `http://${host}/sensor`,
      data: { location, rh, t },
    });
  });
}

module.exports = (loc, ep) => { setInterval(measure, interval, loc, ep); };
