import _ from 'lodash';
import React from 'react';
import {
  Box,
  Card,
  CardBody,
  CardFooter,
  CardHeader,
  DataChart,
  ResponsiveContext,
  Text,
} from 'grommet';
import { IconButton } from 'grommet-controls';
import { Add, Radial, Subtract } from 'grommet-icons';
import GaugeChart from 'react-gauge-chart';
import dayjs from 'dayjs';
import numbro from 'numbro';

numbro.setDefaults({ average: true });

const fmt = (v, f) => (v === undefined || isNaN(v)) ? '--' : numbro(v).format(f);

const Room = (props) => {
  const size = React.useContext(ResponsiveContext);
  const isSmall = size === 'small' || size === 'xsmall';
  const { id, nid, title, data, ts } = props;
  const lb = 19.0, ub = 33.0;
  const t = _.get(data, 't');
  const tp = _.get(data, 'tp');
  let tpct = (t - lb) / (ub - lb);
  if (tpct < -0.13) {
    tpct = -0.13;
  } else if (tpct > 1.13) {
    tpct = 1.13;
  }
  let h = _.get(data, 'h');
  const hpct = h / 100;
  const mkArcs = (v) => {
    if (isNaN(v)) return [0, 1, 0];
    let pl = (v - lb) / (ub - lb) - 0.05;
    let pr = (v - lb) / (ub - lb) + 0.05;
    if (pl < 0) pl = 0; else if (pl > 1) pl = 1;
    if (pr < 0) pr = 0; else if (pr > 1) pr = 1;
    return [pl, pr - pl, 1 - pr];
  };
  return (
    <Card width={{ min: '100px' }} background='light' margin='small' flex>
      <CardHeader pad='small' justify='center' background={id}>{title}</CardHeader>
      <CardBody pad={{ top: 'small' }} flex='grow' align='center'>
        <GaugeChart id={'gauge-' + id + '-t'}
          colors={['#8cb964', '#fae500', '#ca4951']}
          arcsLength={mkArcs(nid !== '0' ? tp : ts)}
          arcWidth={0.5}
          percent={tpct}
          hideText
          animate={false}
        />
        <Box>
          <Text weight='bold'>t<sub>{nid}</sub>: {fmt(t, { mantissa: 2 })}&#8451;</Text>
        </Box>
        <Box flex='grow'>
          {nid !== '0' ? (
            <Text>t<sub>p{nid}</sub>: {fmt(tp, { mantissa: 2 })}&#8451;</Text>
          ) : (
            <Box direction='row' gap='small'>
              <Text>{fmt(_.get(data, 'uvi'), { mantissa: 2 })}</Text>
              <Text>{fmt(_.get(data, 'clouds'), { mantissa: 0, average: true })}%</Text>
              <Text>{fmt(_.get(data, 'wind'), { mantissa: 2 })}m/s</Text>
            </Box>
          )}
        </Box>
        {!isSmall && (
          <GaugeChart id={'gauge-' + id + '-h'}
            colors={['#ca4951', '#fae500', '#8cb964']}
            arcsLength={nid === '1' ? [0.45, 0.40, 0.15] : [0.3, 0.55, 0.15]}
            arcWidth={0.3}
            percent={hpct}
            hideText
            animate={false}
          />
        )}
        <Box>
          <Text weight='bold'>h<sub>{nid}</sub>: {fmt(_.get(data, 'h'), { mantissa: 2 })}%</Text>
        </Box>
      </CardBody>
    </Card>
  );
};

const TempChart = ({ data }) => (
  <Box margin='small' align='center' justify='center'>
    <Box fill='horizontal' align='center' justify='center'>
      <DataChart
        size={{ width: 'fill' }}
        gap='none'
        pad={{ horizontal: 'medium', vertical: 'small' }}
        data={Array.isArray(data) ? data : []}
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
          x: {
            property: 'dt',
            granularity: 'coarse', // TODO: https://github.com/grommet/grommet/issues/4602
          },
          y: {
            property: 't1',
            granularity: 'fine',
          },
        }}
        guide={{
          x: { granularity: 'medium' },
          y: { granularity: 'fine' },
        }}
        detail
      />
    </Box>
  </Box>
);

const HumidChart = ({ data }) => (
  <Box margin='small' align='center' justify='center'>
    <Box fill='horizontal' align='center' justify='center'>
      <DataChart
        size={{ width: 'fill' }}
        gap='none'
        pad={{ horizontal: 'medium', vertical: 'small' }}
        data={Array.isArray(data) ? data : []}
        series={[{
          property: 'dt',
          render: (dt) => (
            <Text margin={{ horizontal: 'xsmall' }}>
              {dayjs(dt).format('HH:mm:ss')}
            </Text>
          ),
        }, 'h1', 'h2']}
        bounds='align'
        chart={[{
          property: 'h1',
          type: 'line',
          thickness: 'hair',
          color: 'room1',
        }, {
          property: 'h2',
          type: 'line',
          thickness: 'hair',
          color: 'room2',
        }]}
        axis={{
          x: {
            property: 'dt',
            granularity: 'coarse', // TODO: https://github.com/grommet/grommet/issues/4602
          },
          y: {
            property: 'h1',
            granularity: 'fine',
          },
        }}
        guide={{
          x: { granularity: 'medium' },
          y: { granularity: 'fine' },
        }}
        detail
      />
    </Box>
  </Box>
);

const Entry = ({ data, label, format }) => (
  <Box direction='row' margin='xxsmall' gap='xxsmall'>
    <Box width='2.5em' align='end'>{label}:</Box>
    <Box width='2.5em' align='start'>{fmt(_.get(data, label), format || { mantissa: 2 })}</Box>
  </Box>
);

const Dashboard = ({ history, current, setOffset2 }) => {
  const size = React.useContext(ResponsiveContext);
  const isSmall = size === 'small' || size === 'xsmall';
  const currSensor = (
    <Box fill='horizontal' direction='row' justify='center'>
      <Room
        id='room1'
        nid='1'
        title='Bedroom'
        data={_.get(current, 'r1')}
      />
      <Room
        id='room2'
        nid='2'
        title='Living Room'
        data={_.get(current, 'r2')}
      />
      <Room
        id='env'
        nid='0'
        title='Environment'
        data={_.get(current, 'r0')}
        ts={(_.get(current, 'r1.tp') + _.get(current, 'r2.tp')) / 2}
      />
    </Box>
  );
  const histSensor = (
    <React.Fragment>
      <TempChart data={history} />
      <HumidChart data={history} />
    </React.Fragment>
  );
  const state = _.get(current, 'f.state') || '(unknown)';
  let slept = _.get(current, 'f.slept') / 60;
  if (slept > 24) slept = 24; if (slept < 0) slept = NaN;
  slept = `${fmt(Math.floor(slept), { characteristic: 2 })}:${fmt((slept - Math.floor(slept)) * 60, { characteristic: 2 })}`
  const other = _.get(current, 'other');
  const currControl = (
    <Box width={isSmall ? '100%' : '260px'} direction='row' wrap={!isSmall} justify='center' margin={isSmall ? { top: '-2px' } : { right: 'small' }} gap='none'>
      <Card width={isSmall ? '120px' : '100%'} background='light' margin='small' flex={isSmall ? false : 'grow'}>
        <CardHeader pad='small' justify='center' background='dark'>F State</CardHeader>
        <CardBody pad='small' justify='center' margin={{ bottom: '-10px' }} flex>
          <Box align='center'>
            <Text weight='bold' color='dark' margin='xsmall' style={{ marginBottom: 0 }}>{state}</Text>
            <Text color='dark' margin='xxsmall'>{slept}</Text>
            <Box direction='horizontal' margin='xxsmall' gap='small' justity='evenly'>
              <Text weight='bold' color='warm'>{fmt(_.get(current, 'f.offset'), { forceSign: true, mantissa: 2 })}&#8451;</Text>
              <Text weight='bold' color='room2'>{fmt(_.get(current, 'f.offset2'), { forceSign: true, mantissa: 2 })}&#8451;</Text>
            </Box>
          </Box>
        </CardBody>
        <CardFooter gap='none' direction='row' backgrund='light' justify='evenly'>
          <IconButton
            margin={isSmall && { top: '-7px', bottom: '-7px', left: '-5px', right: '-5px' }}
            icon={<Subtract color='cold' />}
            hoverIndicator
            onClick={() => { setOffset2 && setOffset2(-1); }}
          />
          <IconButton
            margin={isSmall && { top: '-7px', bottom: '-7px', left: '-5px', right: '-5px' }}
            icon={<Radial color='black' />}
            hoverIndicator
            onClick={() => { setOffset2 && setOffset2(0); }}
          />
          <IconButton
            margin={isSmall && { top: '-7px', bottom: '-7px', left: '-5px', right: '-5px' }}
            icon={<Add color='warm' />}
            hoverIndicator
            onClick={() => { setOffset2 && setOffset2(+1); }}
          />
        </CardFooter>
      </Card>
      <Card width={isSmall ? '120px' : '100%'} height={{ max: isSmall ? '165px' : undefined }} background='light' margin='small' flex='grow'>
        <CardHeader pad='small' justify='center' background='dark'>F/C/H0 output</CardHeader>
        <CardBody direction={isSmall ? 'column' : 'row'} pad='small' wrap justify={isSmall ? 'start' : 'evenly'} overflow={isSmall ? 'scroll' : undefined}>
          <Entry data={other} label='heat' />
          <Entry data={other} label='acm' format={{ forceSign: true }} />
          <Entry data={other} label='acp' />
          <Entry data={other} label='reg1' />
          <Entry data={other} label='reg2' />
          <Entry data={other} label='fan' format={{}} />
          <Entry data={other} label='win' />
          <Entry data={other} label='cur' />
          <Entry data={other} label='w0' format={{ mantissa: 1 }} />
          <Entry data={other} label='w1' format={{ mantissa: 1 }} />
          <Entry data={other} label='w2' format={{ mantissa: 1 }} />
          <Entry data={other} label='curbl' />
          <Entry data={other} label='curbu' />
          <Entry data={other} label='f012' />
          <Entry data={other} label='t0d' />
          <Entry data={other} label='f012bl' />
          <Entry data={other} label='f012bu' />
          <Entry data={other} label='ac1' format={{ forceSign: true, mantissa: 2 }} />
          <Entry data={other} label='ac2' format={{ forceSign: true, mantissa: 2 }} />
          <Entry data={other} label='t1m0' format={{ forceSign: true, totalLength: 3 }} />
          <Entry data={other} label='Wsun' />
          <Entry data={other} label='qest' />
          <Entry data={other} label='cest' />
          <Entry data={other} label='tsr' format={{ forceSign: true, mantissa: 1 }} />
          <Entry data={other} label='tss' format={{ forceSign: true, mantissa: 1 }} />
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
      <Box direction='row' gap='small' align='start'>
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

export default Dashboard;
