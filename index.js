const express = require('express');
const bodyParser = require('body-parser');
const dayjs = require('dayjs');
const { spawn } = require('child_process');
const app = express();

app.use(express.static(__dirname));

const goRight = () => {
  if (process.env.DEBUG) {
    console.log('>>>>>>>>');
  } else {
    spawn('./main.py', ['right']);
  }
};

const goLeft = () => {
  if (process.env.DEBUG) {
    console.log('<<<<<<<<');
  } else {
    spawn('./main.py', ['left']);
  }
};

function getDelay(req, res, next) {
  if (req.body === 'now') {
    req.delay = 0;
  } else if (req.body.startsWith('+')) {
    req.delay = +req.body;
  } else {
    const tn = dayjs();
    let tx = dayjs(tn.format('YYYY-MM-DDT') + req.body, 'YYYY-MM-DDTHH:mm', true);
    if (tx.isBefore(tn)) {
      tx = tx.add(1, 'day');
    }
    req.delay = tx.diff(tn, 's') / 60;
  }
  next();
}

const pending = [];

app.post('/open', bodyParser.text(), getDelay, (req, res) => {
  pending.push(setTimeout(goRight, req.delay * 60 * 1000));
  res.send(`opening ${req.delay} mins later`);
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

const port = process.env.DEBUG ? 3000 : 80;
console.log(`Starting httpd server on 0.0.0.0:${port}`)
app.listen(port);
