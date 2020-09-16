const path = require('path');
const fs = require('fs');
const JSON5 = require('json5');
const axios = require('axios').default;

const interval = 10 * 60e3;
const config = JSON5.parse(fs.readFileSync(path.join(__dirname, '..', 'weather.json5'), 'utf8'));

async function measure(endpoint) {
  const res = await axios({
    method: 'get',
    url: 'https://community-open-weather-map.p.rapidapi.com/weather',
    headers: {
      'X-RapidAPI-Key': config.key,
    },
    params: {
      lat: config.lat,
      lon: config.lon,
    },
    validateStatus: null,
  });
  if (res.status !== 200) {
    console.error(`${res.status} ${res.statusText}`);
    console.error(res.data);
    return;
  }
  if (process.env.DEBUG === 'weather') {
    console.error(res.headers);
    console.error(res.data);
  }
  const { sunrise, sunset } = res.data.sys;
  const sun = sunrise * 1000 <= +new Date() && sunset * 1000 >= +new Date();
  await axios({
    method: 'post',
    url: `http://${endpoint}/weather`,
    data: {
      location: 0,
      rh: res.data.main.humidity,
      t: res.data.main.temp - 273.15,
      wind: res.data.wind.speed,
      sun,
    },
  });
}

module.exports = (endpoint) => {
  if (process.env.DEBUG === 'weather')
    measure(endpoint);
  setInterval(measure, interval, endpoint);
};
