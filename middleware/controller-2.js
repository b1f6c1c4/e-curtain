const path = require('path');
const sensor = require('../backend/sensor');

const port = process.env.DEBUG ? 3000 : 80;

sensor(1, `controller-0:${port}`);
