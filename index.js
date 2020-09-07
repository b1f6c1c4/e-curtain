const path = require('path');
const fs = require('fs');

(() => {
  try {
    fs.accessSync(path.join(__dirname, 'controller-0'), fs.constants.F_OK);
    require('./middleware/controller-0');
    return;
  } catch (e) {
    console.error(e);
  }

  try {
    fs.accessSync(path.join(__dirname, 'controller-1'), fs.constants.F_OK);
    require('./middleware/controller-1');
    return;
  } catch (e) {
    console.error(e);
  }

  try {
    fs.accessSync(path.join(__dirname, 'controller-2'), fs.constants.F_OK);
    require('./middleware/controller-2');
    return;
  } catch (e) {
    console.error(e);
  }

  console.error('Unable to determine the controller.');
  process.exit(3);
})();
