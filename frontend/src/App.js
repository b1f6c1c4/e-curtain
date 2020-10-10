import React, { useState } from 'react';
import { Grommet } from 'grommet';
import ky from 'ky';
import dayjs from 'dayjs';
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

const url = '';

const App = () => {
  const [current, setCurrent] = useState(undefined);
  const [history, setHistory] = useState(undefined);

  useInterval(async () => {
    try {
      const res = await ky.get(url + '/current').json();
      setCurrent(res || null);
    } catch (e) {
      console.error(e);
      setCurrent(null);
    }
  }, current === undefined ? 200 : 10000);
  useInterval(async () => {
    try {
      const res = await ky.get(url + '/history').json();
      res.forEach((v) => {
        v.dt = dayjs(v.dt * 1000).format('YYYY-MM-DDTHH:mm:ss');
      });
      setHistory((res.length > 2 && res) || null);
    } catch (e) {
      console.error(e);
      setHistory(null);
    }
  }, history === undefined ? 200 : 10000);

  const setOffset = async (v) => {
    try {
      await ky.post(url + '/offset', { json: { cmd: v } }).json();
    } catch (e) {
      console.error(e);
    }
  };

  return (
    <Grommet theme={theme} full>
      <Dashboard
        current={current}
        history={history}
        setOffset={setOffset}
      />
    </Grommet>
  );
};

export default App;
