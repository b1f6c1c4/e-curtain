const axios = require('axios').default;
const express = require('express');
const bodyParser = require('body-parser');
const dayjs = require('dayjs');
const isoWeek = require('dayjs/plugin/isoWeek')
const servo = require('../backend/servo');
const rf = require('../backend/rf');

const app = express();
dayjs.extend(isoWeek);

app.post('/sensor', bodyParser.json(), (req, res) => {
  // TODO
});

app.post('/rf', bodyParser.json(), (req, res) => {
  // TODO
});

const port = process.env.DEBUG ? 3000 : 80;
console.log(`Listening on 0.0.0.0:${port}`)
app.listen(port);

rf(`controller-0:${port}`);
