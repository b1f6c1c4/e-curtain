const path = require('path');
const sensor = require('../backend/sensor');

const port = process.env.DEBUG ? 3000 : 80;

if (!process.env.DEBUG) {
  sensor(2, 'controller-0:80');
} else {
  sensor(2, 'localhost:3000');
}
