const path = require('path');
const express = require('express');
const bodyParser = require('body-parser');
const dayjs = require('dayjs');
const { spawn } = require('child_process');
const isoWeek = require('dayjs/plugin/isoWeek')
dayjs.extend(isoWeek);
const app = express();

app.use(express.static(__dirname));

const goRight = () => {
  if (process.env.DEBUG) {
    console.log('>>>>>>>>');
  } else {
    spawn(path.join(__dirname, 'curtain.py'), ['right']);
  }
};

const goLeft = () => {
  if (process.env.DEBUG) {
    console.log('<<<<<<<<');
  } else {
    spawn(path.join(__dirname, 'curtain.py'), ['left']);
  }
};

const getTime = (str) => {
  const tn = dayjs();
  let tx = dayjs(tn.format('YYYY-MM-DDT') + str, 'YYYY-MM-DDTHH:mm', true);
  if (tx.isBefore(tn)) {
    tx = tx.add(1, 'day');
  }
  return tx;
};

function getDelay(req, res, next) {
  if (req.body === 'now') {
    req.delay = 0;
  } else if (req.body.startsWith('+')) {
    req.delay = +req.body;
  } else {
    req.delay = getTime(req.body).diff(dayjs(), 's') / 60;
  }
  next();
}

let tomorrowMorning = null;
let morning = null;
const pending = [];

app.post('/open', bodyParser.text(), getDelay, (req, res) => {
  if (req.body === '?') {
    if (!tomorrowMorning) {
      res.send('tomorrow no morning open');
    } else {
      delay = tomorrowMorning.diff(dayjs(), 's') / 60;
      res.send(`tomorrow will open ${tomorrowMorning.format('HH:mm')}, nearest of which is ${delay} mins later (${tomorrowMorning.format('YYYY-MM-DDTHH:mm (ddd)')})`);
    }
  } else if (req.body === '??') {
    if (!morning) {
      res.send('no morning open');
    } else {
      delay = morning.diff(dayjs(), 's') / 60;
      res.send(`open every ${morning.format('HH:mm')}, nearest of which is ${morning.format('YYYY-MM-DDTHH:mm (ddd)')}`);
    }
  } else if (req.body === '==0') {
    tomorrowMorning = morning = null;
    res.send('no more morning open');
  } else if (req.body === '=0') {
    tomorrowMorning = null;
    res.send('tomorrow no morning open');
  } else if (req.body.startsWith('==')) {
    morning = tomorrowMorning = getTime(req.body.substr(2));
    if (morning.isoWeekday() <= 4) {
      morning = morning.add(1, 'day');
    } else {
      morning = morning.add(8 - morning.isoWeekday(), 'day');
    }
    delay = tomorrowMorning.diff(dayjs(), 's') / 60;
    res.send(`open every ${tomorrowMorning.format('HH:mm')}, nearest of which is ${delay} mins later (${tomorrowMorning.format('YYYY-MM-DDTHH:mm (ddd)')})`);
  } else if (req.body.startsWith('=')) {
    tomorrowMorning = getTime(req.body.substr(1));
    delay = tomorrowMorning.diff(dayjs(), 's') / 60;
    res.send(`tomorrow will open ${tomorrowMorning.format('HH:mm')}, nearest of which is ${delay} mins later`);
  } else {
    pending.push(setTimeout(goRight, req.delay * 60 * 1000));
    res.send(`opening ${req.delay} mins later`);
  }
});

app.post('/close', bodyParser.text(), getDelay, (req, res) => {
  pending.push(setTimeout(goLeft, req.delay * 60 * 1000));
  res.send(`closing ${req.delay} mins later`);
});

app.post('/cancel', (req, res) => {
  pending.forEach((v) => { clearTimeout(v); });
  pending.length = 0;
  res.send('cancelled');
});

setInterval(() => {
  if (tomorrowMorning) {
    if (dayjs().isAfter(tomorrowMorning)) {
      goRight();
      tomorrowMorning = null;
    }
  }
  if (tomorrowMorning || !morning) return;
  const d = morning.diff(dayjs(), 'd');
  if (d < 1.2) {
    tomorrowMorning = morning;
    if (morning.isoWeekday() <= 4) {
      morning = morning.add(1, 'day');
    } else {
      morning = morning.add(8 - morning.isoWeekday(), 'day');
    }
    console.log(`tmr=${tomorrowMorning.format('YYYY-MM-DDTHH:mm')}`);
    console.log(`mr=${morning.format('YYYY-MM-DDTHH:mm')}`);
  }
}, 1000);

const port = process.env.DEBUG ? 3000 : 80;
console.log(`Starting httpd server on 0.0.0.0:${port}`)
app.listen(port);
