const path = require('path');
const axios = require('axios').default;
const express = require('express');
const bodyParser = require('body-parser');
const dayjs = require('dayjs');
const isoWeek = require('dayjs/plugin/isoWeek')
const Datastore = require('nedb');
const servo = require('../backend/servo');
const rf = require('../backend/rf');
const weather = require('../backend/weather');

const filename = path.join(__dirname, '..', 'e-curtain.db')
const db = new Datastore({
  filename,
  autoload: true,
  timestampData: true,
});
db.persistence.setAutocompactionInterval(3.6e6);
db.ensureIndex({
  fieldName: 'createdAt',
});

const app = express();
dayjs.extend(isoWeek);

app.post('/sensor', bodyParser.json(), (req, res) => {
  db.insert(req.body, (err, doc) => {
    if (err) {
      res.status(500).send(err);
    } else {
      res.status(204).send();
      if (process.env.DEBUG) {
        console.log(doc);
      }
    }
  });
});

app.post('/weather', bodyParser.json(), (req, res) => {
  db.insert(req.body, (err, doc) => {
    if (err) {
      res.status(500).send(err);
    } else {
      res.status(204).send();
      if (process.env.DEBUG) {
        console.log(doc);
      }
    }
  });
});

// List of states
// b : Balanced
// w : Working
// wfc : Working, forced cooling
// s0 : Pre-sleep
// s0e : Pre-sleep, extended
// s1 : Entering sleep
// s1p2 : Sleep 20%
// s1p2fh : Sleep 20%, forced heating
// s1p4 : Sleep 40%
// s1p4fh : Sleep 40%, forced heating
// s1p6 : Sleep 60%
// s1p6fh : Sleep 60%, forced heating
// s1p8 : Sleep 80%
// s1p8fh : Sleep 80%, forced heating
// s2 : Exiting sleep
let state;
let vstate;

let rfOld = null, rfT = null;
app.post('/rf', bodyParser.json(), (req, res) => {
  if (+new Date() - rfT > 3e3) {
    rfOld = null;
    rfT = null;
  }
  rfT = +new Date();
  if (req.body.v === rfOld) {
    res.status(429).send();
    return;
  }
  rfOld = req.body.v;

  switch (req.body.v) {
    case 8: // A: wake up
      state = 'w';
      vstate = true;
      break;
    case 4: // B: go to bed, (re)start 8hr timer

    case 2: // C: forced ventilation on/off
      vstate = ~vstate;
      break;
    case 1: // D: forced heating (when asleep) / cooling (when awake) on/off
    default:
      res.status(422).send();
      return;
  }

  // TODO: actuator
  console.log(`${state} ${vstate}`);
});

const port = process.env.DEBUG ? 3000 : 80;
console.log(`Listening on 0.0.0.0:${port}`)
app.listen(port);

if (!process.env.DEBUG) {
  rf(`controller-0:${port}`);
  weather(`controller-0:${port}`);
} else {
  weather(`localhost:${port}`);
}
