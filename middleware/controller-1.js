const path = require('path');
const express = require('express');
const bodyParser = require('body-parser');
const sensor = require('../backend/sensor');
const curtain = require('../backend/curtain');

app.post('/open', bodyParser.text(), getDelay, (req, res) => {
  curtain(true);
  res.status(204).send();
});

app.post('/close', bodyParser.text(), getDelay, (req, res) => {
  curtain(false);
  res.status(204).send();
});

const port = process.env.DEBUG ? 3000 : 80;
console.log(`Listening on 0.0.0.0:${port}`)
app.listen(port);

sensor(1, `controller-0:${port}`);
rf(`controller-0:${port}`);
