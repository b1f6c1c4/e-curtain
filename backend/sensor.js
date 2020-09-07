const path = require('path');
const axios = require('axios').default;
const shell = require('shelljs');

const interval = 60e3;

function measure(location, endpoint) {
  console.log(`Info: measuring ${location} to ${endpoint}`);
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
      url: `http://${endpoint}/sensor`,
      data: { location, rh, t },
    });
  });
}

module.exports = (loc, ep) => {
  measure(loc, ep);
  setInterval(measure, interval, loc, ep);
};
