import React, { useState } from 'react';
import { Grommet } from 'grommet';
import ky from 'ky';
import Dashboard from './Dashboard';
import { useInterval } from '@react-corekit/use-interval';

const theme = {
  global: {
    colors: {
      room1: '#dfcc74',
      room2: '#64b6ac',
      cold: '#105284',
      warm: '#842469',
      env: '#1b4353',
      light: '#f2f2f2',
      dark: '#141414',
    },
    font: {
      family: 'Roboto',
      size: '18px',
      height: '20px',
    },
  },
};

const App = () => {
  const [current, setCurrent] = useState(undefined);
  const [history, setHistory] = useState(undefined);

  useInterval(async () => {
    try {
      setCurrent((await ky.get('/current')).json());
    } catch (e) {
      console.error(e);
      setCurrent(undefined);
    }
  }, 5000);
  useInterval(async () => {
    try {
      setHistory((await ky.get('/history')).json());
    } catch (e) {
      console.error(e);
      setCurrent(undefined);
    }
  }, 5000);

  return (
    <Grommet theme={theme} full>
      <Dashboard
        current={current}
        history={history}
      />
    </Grommet>
  );
};

export default App;
