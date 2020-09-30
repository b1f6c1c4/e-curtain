#!/usr/bin/node

const axios = require('axios').default;
const shelljs = require('shelljs');
const dgram = require('dgram');
const client = dgram.createSocket('udp4');

if (process.argv.length !== 7) {
  console.error('Usage: set-op.js <f012> <f12> <ac> <host> <port>');
  process.exit(1);
}

const sleep = (t) => new Promise((resolve) => {
  setTimeout(() => { resolve(); }, t);
});

async function run() {
  let f012 = +process.argv[2];
  const f12 = +process.argv[3];
  const ac = +process.argv[4];
  if (isNaN(f012)) {
    f012 = +process.argv[2].substr(1);
  } else {
    await axios({
      method: 'post',
      url: `http://192.168.1.115/${f012 ? 'open' : 'close'}`,
    });
    await axios({
      method: 'post',
      url: `http://192.168.1.115/${f012 ? 'wopen' : 'wclose'}`,
    });
    await axios({
      method: 'post',
      url: `http://192.168.1.115/${f012 ? 'on' : 'off'}`,
    });
    await axios({
      method: 'post',
      url: `http://192.168.1.115/${f012 ? 'on' : 'off'}`,
    });
  }
  let reg2 = 1, reg1 = 1;
  if (!ac) {
    reg2 = 0;
    if (!f12) {
      reg1 = 0;
    }
  } else if (f12 >= 1) {
    reg2 = 2 - f12;
  } else {
    reg1 = f12;
  }
  console.log({ f012, f12, reg1, reg2, ac });
  shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 16 ${75 * reg2}`, () => {});
  await sleep(4000);
  shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 8 ${95 * reg1}`, () => {});
  await sleep(4000);
  if (ac >= +2) {
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 10 0`, () => {});
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 12 0`, () => {});
  } else if (ac >= +1) {
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 10 30`, () => {});
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 12 0`, () => {});
  } else if (ac <= -2) {
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 10 180`, () => {});
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 12 180`, () => {});
  } else if (ac <= -1) {
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 10 150`, () => {});
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 12 180`, () => {});
  } else if (f12 >= 2) {
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 10 120`, () => {});
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 12 90`, () => {});
  } else if (f12 >= 1) {
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 10 90`, () => {});
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 12 90`, () => {});
  } else {
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 10 60`, () => {});
    shelljs.exec(`ssh pi@192.168.1.118 ./e-curtain/backend/servo.py 12 90`, () => {});
  }
  const buf = Buffer.alloc(8 * 5);
  buf.writeDoubleLE(3, 0);
  buf.writeDoubleLE(f012, 8);
  buf.writeDoubleLE(f12, 16);
  buf.writeDoubleLE(ac, 24);
  buf.writeDoubleLE(0, 32);
  client.send(buf, +process.argv[6], process.argv[5], () => { client.close() });
}

run();
