const path = require('path');
const axios = require('axios').default;
const express = require('express');
const bodyParser = require('body-parser');
const dayjs = require('dayjs');
const isoWeek = require('dayjs/plugin/isoWeek')
const Datastore = require('nedb');
const servo = require('../backend/servo');
const slowServo = require('../backend/slow-servo');
const rf = require('../backend/rf');
const weather = require('../backend/weather');
const core = require('./core');

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

const getS = async () => {
  const getLast = (location) => new Promise((resolve) => {
    db.find({ location }).sort({ createdAt: -1 }).limit(1).exec((err, docs) => {
      if (err) {
        console.error(err);
        resolve(null);
      } else if (docs.length) {
        resolve(docs[0]);
      } else {
        console.error(`Error: no sensor data found for ${location}`);
        resolve(null);
      }
    });
  });
  const getAverage = (location, since) => new Promise((resolve) => {
    db.find({ location, createdAt: { $gte: new Date(since) } }).exec((err, docs) => {
      if (err) {
        console.error(err);
        resolve(null);
      } else if (docs.length) {
        const res = { location, rh: 0, t: 0 };
        docs.forEach(({ rh, t }) => { res.rh += rh; res.t += t; });
        res.rh /= docs.length;
        res.t /= docs.length;
        resolve(res);
      } else {
        console.error(`Warning: no recent sensor data found for ${location} since ${since}`);
        resolve(null);
      }
    });
  });
  const s = [];
  const since = +new Date() - 2 * 60e3;
  s[0] = await getLast(0);
  s[1] = await getAverage(1, since) || await getLast(1);
  s[2] = await getAverage(2, since) || await getLast(2);
  return s;
};

const to08 = 0000;
const to16 = 0000;
const to10 = 4000;
const to12 = 4000;

let ores = {};
const tick = async () => {
  const s = await getS();
  if (!(s[0] && s[1] && s[2])) {
    return;
  }
  const res = core.tick(s);
  if (res.fan !== ores.fan) {
    console.log(`Set fan to ${res.fan}`);
    if (!process.env.DEBUG) {
      setTimeout(() => {
        axios({
          method: 'post',
          url: `http://controller-1:80/${res.fan ? 'on' : 'off'}`,
        });
      }, 2000);
    }
  }
  if (res.windows !== ores.windows) {
    console.log(`Set windows to ${res.windows}`);
    if (!process.env.DEBUG) {
      axios({
        method: 'post',
        url: `http://controller-1:80/${res.windows ? 'wopen' : 'wclose'}`,
      });
    }
  }
  if (res.curtain !== ores.curtain) {
    console.log(`Set curtain to ${res.curtain}`);
    if (!process.env.DEBUG) {
      axios({
        method: 'post',
        url: `http://controller-1:80/${res.curtain ? 'open' : 'close'}`,
      });
    }
  }
  if (res.register1 !== ores.register1) {
    console.log(`Set register1 to ${res.register1}`);
    if (!process.env.DEBUG) {
      setTimeout(() => { servo(8, 100 - res.register1 * (100 - 0)); }, to08);
    }
  }
  if (res.register2 !== ores.register2) {
    console.log(`Set register2 to ${res.register2}`);
    if (!process.env.DEBUG) {
      setTimeout(() => { slowServo(res.register2); }, to16);
    }
  }
  if (res.ac !== ores.ac || res.acFan !== ores.acFan) {
    if (res.ac === +1) {
      console.log(`Set ac to heat I`);
      if (!process.env.DEBUG) {
        setTimeout(() => { servo(10, 30); }, to10);
        setTimeout(() => { servo(12, 10); }, to12);
      }
    } else if (res.ac === +2) {
      console.log(`Set ac to heat II`);
      if (!process.env.DEBUG) {
        setTimeout(() => { servo(10, 0); }, to10);
        setTimeout(() => { servo(12, 10); }, to12);
      }
    } else if (res.ac === -1) {
      console.log(`Set ac to cool I`);
      if (!process.env.DEBUG) {
        setTimeout(() => { servo(10, 150); }, to10);
        setTimeout(() => { servo(12, 170); }, to12);
      }
    } else if (res.ac === -2) {
      console.log(`Set ac to cool II`);
      if (!process.env.DEBUG) {
        setTimeout(() => { servo(10, 180); }, to10);
        setTimeout(() => { servo(12, 170); }, to12);
      }
    } else if (res.acFan === +1) {
      console.log(`Set ac to fan I`);
      if (!process.env.DEBUG) {
        setTimeout(() => { servo(10, 90); }, to10);
        setTimeout(() => { servo(12, 0); }, to12);
      }
    } else if (res.acFan === +2) {
      console.log(`Set ac to fan I`);
      if (!process.env.DEBUG) {
        setTimeout(() => { servo(10, 120); }, to10);
        setTimeout(() => { servo(12, 0); }, to12);
      }
    } else {
      console.log(`Set ac to Off`);
      if (!process.env.DEBUG) {
        setTimeout(() => { servo(10, 60); }, to10);
        setTimeout(() => { servo(12, 180); }, to12);
      }
    }
  }
  ores = res;
};

const app = express();
dayjs.extend(isoWeek);

app.get('/state', bodyParser.json(), (req, res) => {
  getS().then((s) => {
    res.json({
      s,
      c: core.state(),
      m: core.mstate(),
      a: ores,
    });
  });
});

app.post('/cmd', bodyParser.json(), (req, res) => {
  console.log(`Info: got cmd data at ${dayjs().toISOString()}`, req.body);
  core.command(req.body.cmd);
  res.status(204).send();
  setImmediate(tick);
});

app.post('/sensor', bodyParser.json(), (req, res) => {
  console.log(`Info: got sensor ${req.body.location} data at ${dayjs().toISOString()}`, req.body);
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
  console.log(`Info: got weather data at ${dayjs().toISOString()}`, req.body);
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

app.post('/rf', bodyParser.json(), (req, res) => {
  console.log(`Info: got rf data ${req.body.v} at ${dayjs().toISOString()}`);
  switch (req.body.v) {
    case 8:
      core.command('A');
      break;
    case 4:
      core.command('B');
      break;
    case 2:
      core.command('C');
      break;
    case 1:
      core.command('D');
      break;
    default:
      res.status(422).send();
      return;
  }
  res.status(204).send();
  setImmediate(tick);
});

core.init();
tick();

setInterval(tick, process.env.DEBUG ? 100 : 30000);

const port = process.env.DEBUG ? 3000 : 80;
console.log(`Listening on 0.0.0.0:${port}`)
app.listen(port);

if (!process.env.DEBUG) {
  rf('controller-0:80');
  weather('controller-0:80');
} else {
  weather('localhost:3000');
}
