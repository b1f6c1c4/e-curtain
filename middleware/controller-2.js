const path = require('path');
const axios = require('axios');
const shell = require('shelljs');

shell.exec(path.join(__dirname, '..', 'backend', 'sensor.py'));
