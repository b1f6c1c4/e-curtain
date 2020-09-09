const path = require('path');
const express = require('express');
const sensor = require('../backend/sensor');
const curtain = require('../backend/curtain');
const relay = require('../backend/relay');

const app = express();

app.post('/open', (req, res) => {
  curtain(true);
  res.status(204).send();
});

app.post('/close', (req, res) => {
  curtain(false);
  res.status(204).send();
});

app.post('/on', (req, res) => {
  relay(true);
  res.status(204).send();
});

app.post('/off', (req, res) => {
  relay(false);
  res.status(204).send();
});

const port = process.env.DEBUG ? 3000 : 80;
console.log(`Listening on 0.0.0.0:${port}`)
app.listen(port);

if (!process.env.DEBUG) {
  sensor(1, 'controller-0:80');
} else {
  sensor(1, 'localhost:3000');
}
