import React, { useState } from 'react';
import {
  Box,
  Card,
  CardBody,
  CardFooter,
  CardHeader,
  DataChart,
  Grommet,
  RangeSelector,
  ResponsiveContext,
  Text,
} from 'grommet';
import { IconButton } from 'grommet-controls';
import { Add, Radial, Subtract } from 'grommet-icons';
import GaugeChart from 'react-gauge-chart';
import dayjs from 'dayjs';

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

const Room = (props) => {
  const size = React.useContext(ResponsiveContext);
  const isSmall = size === 'small' || size === 'xsmall';
  const { id, nid, title } = props;
  return (
    <Card width={{ min: '100px' }} background='light' margin='small' flex>
      <CardHeader pad='small' justify='center' background={id}>{title}</CardHeader>
      <CardBody pad={{ top: 'small' }} flex='grow' align='center'>
        <GaugeChart id={'gauge-' + id + '-t'}
          colors={['#8cb964', '#fae500', '#ca4951']}
          arcsLength={[0.4, 0.1, 0.4]}
          arcWidth={0.5}
          percent={0.37}
          hideText
          animate={false}
        />
        <Box>
          <Text weight='bold'>t<sub>{nid}</sub>: 37.11&#8451;</Text>
        </Box>
        <Box flex='grow'>
          {nid !== '0' ? (
            <Text>t<sub>p{nid}</sub>: 37.11&#8451;</Text>
          ) : (
            <Box direction='row' gap='small'>
              <Text>4.11</Text>
              <Text>3m/s</Text>
            </Box>
          )}
        </Box>
        {!isSmall && (
          <GaugeChart id={'gauge-' + id + '-h'}
            colors={['#8cb964', '#fae500', '#ca4951']}
            arcsLength={[0.3, 0.6, 0.1]}
            arcWidth={0.3}
            percent={0.37}
            hideText
            animate={false}
          />
        )}
        <Box>
          <Text weight='bold'>h<sub>{nid}</sub>: 65.43%</Text>
        </Box>
      </CardBody>
    </Card>
  );
};

const Chart = () => {
  const [range, setRange] = useState([0, 24]);
  const onChange = (values) => {
    setRange(values);
  };
  return (
    <Box margin='small' align='center' justify='center'>
      <Box fill='horizontal' align='center' justify='center'>
        <DataChart
          size={{ width: 'fill' }}
          gap='none'
          pad={{ horizontal: 'medium', vertical: 'small' }}
          data={[
            { dt: '2020-07-01T12:23', t1: 12, t2: 13, tp1: 11, tp2: 14 },
            { dt: '2020-07-01T12:33', t1: 13, t2: 13, tp1: 11, tp2: 14 },
            { dt: '2020-07-01T12:43', t1: 12, t2: 14, tp1: 11, tp2: 14 },
            { dt: '2020-07-01T12:53', t1: 12, t2: 13, tp1: 12, tp2: 15 },
          ]}
          series={[{
            property: 'dt',
            render: (dt) => (
              <Text margin={{ horizontal: 'xsmall' }}>
                {dayjs(dt).format('HH:mm:ss')}
              </Text>
            ),
          }, 't1', 't2', 'tp1', 'tp2']}
          bounds='align'
          chart={[{
            property: 't1',
            type: 'line',
            thickness: 'hair',
            color: 'room1',
          }, {
            property: 'tp1',
            type: 'line',
            thickness: 'xxsmall',
            color: 'room1',
            dash: true,
          }, {
            property: 't2',
            type: 'line',
            thickness: 'hair',
            color: 'room2',
          }, {
            property: 'tp2',
            type: 'line',
            thickness: 'xxsmall',
            color: 'room2',
            dash: true,
          }]}
          axis={{
            x: { property: 'dt' },
            y: {
              property: 't1',
              ganularity: 'fine',
            },
          }}
          guide={{
            x: { granularity: 'fine' },
            y: { granularity: 'fine' },
          }}
          detail
        />
      </Box>
      <Box fill='horizontal' pad={{ left: 'small', right: 'small' }}>
        <RangeSelector
          direction='horizontal'
          min={0}
          max={24}
          values={range}
          onChange={onChange}
        />
      </Box>
    </Box>
  );
};

const Entry = ({ label, children }) => (
  <Box direction='row' margin='xxsmall' gap='xxsmall'>
    <Box width='2.5em' align='end'>{label}:</Box>
    <Box width='2.5em' align='start'>{children}</Box>
  </Box>
);

const Page = () => {
  // const [showSidebar, setShowSidebar] = useState(false);
  const size = React.useContext(ResponsiveContext);
  const isSmall = size === 'small' || size === 'xsmall';
  const currSensor = (
    <Box fill='horizontal' direction='row' justify='center'>
      <Room
        id='room1'
        nid='1'
        title='Bedroom'
      />
      <Room
        id='room2'
        nid='2'
        title='Living Room'
      />
      <Room
        id='env'
        nid='0'
        title='Environment'
      />
    </Box>
  );
  const histSensor = (
    <Chart />
  );
  const currControl = (
    <Box width={isSmall ? '100%' : '260px'} direction='row' wrap={!isSmall} justify='center' margin={isSmall ? { top: '-2px' } : { right: 'small' }} gap='none'>
      <Card width={isSmall ? '120px' : '100%'} background='light' margin='small' flex={isSmall ? false : 'grow'}>
        <CardHeader pad='small' justify='center' background='dark'>F State</CardHeader>
        <CardBody pad='small' justify='center' margin={{ bottom: '-10px' }} flex>
          <Box align='center'>
            <Text weight='bold' color='dark' margin='xsmall' style={{ marginBottom: 0 }}>NORMAL</Text>
            <Text color='dark' margin='xxsmall'>04:24</Text>
            <Text weight='bold' color='warm' margin='xxsmall'>+0.3&#8451;</Text>
          </Box>
        </CardBody>
        <CardFooter gap='none' direction='row' backgrund='light' justify='evenly'>
          <IconButton margin={isSmall && { top: '-7px', bottom: '-7px', left: '-5px', right: '-5px' }} icon={<Subtract color='cold' />} hoverIndicator />
          <IconButton margin={isSmall && { top: '-7px', bottom: '-7px', left: '-5px', right: '-5px' }} icon={<Radial color='black' />} hoverIndicator />
          <IconButton margin={isSmall && { top: '-7px', bottom: '-7px', left: '-5px', right: '-5px' }} icon={<Add color='warm' />} hoverIndicator />
        </CardFooter>
      </Card>
      <Card width={isSmall ? '120px' : '100%'} height={{ max: isSmall ? '165px' : undefined }} background='light' margin='small' flex='grow'>
        <CardHeader pad='small' justify='center' background='dark'>F/C/H0 output</CardHeader>
        <CardBody direction={isSmall ? 'column' : 'row'} pad='small' wrap justify={isSmall ? 'start' : 'evenly'} overflow={isSmall ? 'scroll' : undefined}>
          <Entry label='acm'>+2</Entry>
          <Entry label='acp'>0.12</Entry>
          <Entry label='reg1'>0.12</Entry>
          <Entry label='reg2'>0.12</Entry>
          <Entry label='fan'>1</Entry>
          <Entry label='win'>0.3</Entry>
          <Entry label='cur'>1</Entry>
          <Entry label='w0'>1.0</Entry>
          <Entry label='w1'>1.0</Entry>
          <Entry label='w2'>1.0</Entry>
          <Entry label='curbl'>1</Entry>
          <Entry label='curbu'>1</Entry>
          <Entry label='f012'>0.32</Entry>
          <Entry label='f012bl'>1</Entry>
          <Entry label='f012bu'>1</Entry>
          <Entry label='ac1'>+0.32</Entry>
          <Entry label='ac2'>-0.32</Entry>
          <Entry label='t1m0'>-30</Entry>
          <Entry label='Wsun'>1502</Entry>
          <Entry label='qest'>15.3</Entry>
          <Entry label='cest'>14.3</Entry>
          <Entry label='t0d'>14.3</Entry>
          <Entry label='tsr'>1.0</Entry>
          <Entry label='tss'>1.0</Entry>
        </CardBody>
      </Card>
    </Box>
  );
  let obj;
  if (isSmall) {
    obj = (
      <Box>
        {currSensor}
        {currControl}
        {histSensor}
      </Box>
    );
  } else {
    obj = (
      <Box direction='row' gap='small'>
        <Box width='700px' flex>
          {currSensor}
          {histSensor}
        </Box>
        {currControl}
      </Box>
    );
  }
  return obj;
}

const App = () => (
  <Grommet theme={theme} full>
    <Page />
  </Grommet>
);

export default App;
