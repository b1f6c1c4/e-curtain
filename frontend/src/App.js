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
  }, current === undefined ? 200 : 1000);
  useInterval(async () => {
    try {
      const res = await ky.get(url + '/history', {
        searchParams: {
          since: +new Date() - 60*60*1000,
          until: +new Date(),
        },
      }).json();
      res.forEach((v) => {
        v.dt = dayjs(v.dt).format('YYYY-MM-DDTHH:mm:ss');
      });
      setHistory((res.length > 2 && res) || null);
    } catch (e) {
      console.error(e);
      setHistory(null);
    }
  }, history === undefined ? 200 : 1000);

  const setOffset2 = async (v) => {
    try {
      await ky.post(url + '/offset2', { json: { cmd: v } }).json();
    } catch (e) {
      console.error(e);
    }
  };

  return (
    <Grommet theme={theme} full>
      <Dashboard
        current={current}
        history={history}
        setOffset2={setOffset2}
      />
    </Grommet>
  );
};

export default App;
