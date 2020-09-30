#!/usr/bin/node

const axios = require('axios').default;
const dgram = require('dgram');
const client = dgram.createSocket('udp4');

if (process.argv.length !== 4) {
  console.error('Usage: fetch-t0.js <host> <port>');
  process.exit(1);
}

axios({
  method: 'get',
  url: `https://community-open-weather-map.p.rapidapi.com/weather?lat=${process.env.LAT}&lon=${process.env.LON}`,
  headers: {
    'X-RapidAPI-Key': process.env.API_KEY,
  },
}).then(({ data }) => {
  const t0 = data.main.temp - 273.15;
  const h0 = data.main.humidity;
  const buf = Buffer.alloc(8 * 5);
  buf.writeDoubleLE(0, 0);
  buf.writeDoubleLE(t0, 8);
  buf.writeDoubleLE(h0, 16);
  buf.writeDoubleLE(0, 24);
  buf.writeDoubleLE(0, 32);
  console.log(buf);
  client.send(buf, +process.argv[3], process.argv[2], () => { client.close() });
});
