const dayjs = require('dayjs');

// List of states
// b : Balanced
// w : Waking up
// n : Normal
// s : Sleeping
// sx : Re-sleeping
let state;
let nstate, ddl;
let vcnt;

const min = process.env.DEBUG ? 0.2e3 : 60e3;
let offset = 0;
let tt, tc;
let k;
let denoised;
let novent;

module.exports = {
  init: () => {
    state = { s: 'b', v: false };
    nstate = null;
    ddl = null;
    vcnt = 0;
    offset = 0;
  },
  state: () => ({
    state,
    nstate,
    ddl,
    vcnt,
    offset,
  }),
  mstate: () => ({
    tt,
    tc,
    k,
    novent,
    denoised,
  }),
  command: (cmd) => {
    switch (cmd) {
      case 'A': // waking up
        state = { s: 'w', v: true };
        nstate = { s: 'n', v: false };
        ddl = +new Date() + 15 * min;
        break;
      case 'B': // go to bed, (re)start 8hr timer
        if (state.p !== undefined && state.p > 180) {
          state = { s: 'sx', p: state.p - 60, v: true };
          nstate = { s: 's', p: state.p - 60 + 15, v: false };
          ddl = +new Date() + 15 * min;
        } else {
          state = { s: 's', v: true };
          nstate = { s: 's', p: 0, v: false };
          ddl = +new Date() + 5 * min;
        }
        break;
      case 'C':
        offset += 0.5;
        break;
      case 'D':
        offset -= 0.5;
        break;
      case 'b':
        state = { s: 'n', v: false };
        nstate = null;
        ddl = null;
        break;
      case 'n':
        state = { s: 'n', v: false };
        nstate = { s: 'n', v: true };
        ddl = +new Date() + 15 * min;
        break;
      case 's':
        state = { s: 's', p: 0, v: false };
        nstate = { s: 's', p: 5, v: false };
        ddl = +new Date() + 5 * min;
        break;
    }
    if (state.v) vcnt = 0;
    console.log(
      dayjs().toISOString(),
      'Cmd:', cmd,
      '->', state,
      '->', nstate,
      'ddl:', ddl && dayjs(ddl).toISOString(),
      'vcnt:', vcnt,
      'offset:', offset,
    );
  },
  tick: (s) => {
    novent = !!state.novent;
    novent |= Math.abs(s[0].t - (s[1] + s[2]) / 2) > 5 + 5 * vcnt;
    novent |= s[0].wind > 5.5;
    if (nstate) {
      const slack = +new Date() - ddl;
      if (slack >= 0) {
        const ostate = state;
        state = nstate;
        switch (state.s) {
          case 'n':
            nstate = { s: state.s, v: !state.v };
            ddl = +new Date() + (state.v ? 5 : 50) * min;
            break;
          case 's':
            nstate = { s: state.s, p: state.p + 5, v: false };
            ddl = +new Date() + 5 * min;
            break;
        }
        if (state.v) {
          if (novent) {
            vcnt++;
            state.v = false;
          } else {
            vcnt = 0;
          }
        }
        console.log(
          dayjs().toISOString(),
          'State:', ostate,
          '->', state,
          '->', nstate,
          'ddl:', ddl && dayjs(ddl).toISOString(),
          'vcnt:', vcnt,
        );
      }
    }

    switch (state.s) {
      case 'b':
        tt = { min: 23, max: 24 };
        break;
      case 'w':
      case 'n':
        tt = { min: 22, max: 24 };
        break;
      case 's':
        if (state.p === undefined) {
          tt = { min: 24, max: 26 };
        } else if (state.p < 120) {
          tt = { min: 24, max: 26 };
        } else if (state.p < 120 + 240) {
          tt = { min: 24 - (state.p - 120) / 60, max: 26 - (state.p - 120) / 60 };
        } else if (state.p < 120 + 240 + 120) {
          tt = { min: 20, max: 22 };
        } else {
          tt = { min: 23, max: 25 };
        }
        break;
      case 'sx':
        tt = { min: 26, max: 28 };
        break;
    }
    tt.min += offset;
    tt.max += offset;
    switch (state.s) {
      case 'b':
        k = 1 / 3;
        break;
      case 'w':
        k = 2 / 3;
        break;
      case 'n':
        k = 0;
        break;
      case 's':
      case 'sx':
        k = 1;
        break;
    }
    tc = k*s[1].t + (1-k)*s[2].t;
    switch (state.s) {
      case 'b':
        denoised = true;
        break;
      case 'w':
      case 'n':
        denoised = false;
        break;
      case 's':
      case 'sx':
        denoised = true;
        break;
    }
    denoised |= state.novent;

    let f012 = 0;
    let ac = 0, acFan = 0;

    if (tc > (tt.max - tt.min) * 2 / 3 + tt.min) {
      if (tt.min - s[0].t >= 3 * (tc - tt.max) && !denoised) {
        f012 = 1;
      } else if (tc > tt.max) {
        ac = -2;
      } else if (tc > (tt.max - tt.min) * 3 / 4 + tt.min) {
        ac = -1;
      }
      if (state.v && s[0].t > tt.max) {
        ac -= 1;
      }
    } else if (tc < (tt.max - tt.min) * 1 / 3 + tt.min) {
      if (s[0].t - tt.max >= 3 * (tt.min - tc) && !denoised) {
        f012 = 1;
      } else if (tc < tt.min) {
        ac = +2;
      } else if (tc < (tt.max - tt.min) * 1 / 5 + tt.min) {
        ac = +1;
      }
      if (state.v && s[0].t < tt.min) {
        ac += 1;
      }
    }
    if (ac > +2)
      ac = +2;
    if (ac < -2)
      ac = -2;

    let register = 0;
    if (ac !== 0) {
      register = k;
    } else if (state.s === 's' || state.s === 'sx') {
      register = 1;
      acFan = 2;
    }

    if (ac === 0 && f012) {
      acFan = 2;
    }

    let fan = false;
    fan |= state.v;
    fan |= !!f012;
    let windows = true;
    windows &= Math.abs(s[0].t - s[1].t) < 2;
    windows &= Math.abs(s[0].t - s[2].t) < 2;
    windows &= Math.abs(s[0].t - tc) < 2;
    windows &= Math.abs(s[0].t - (tt.max + tt.min) / 2) < 2;
    windows &= s[0].wind < 4.5;
    windows &= !(state.s === 's' || state.s === 'sx');
    windows |= fan;
    let curtain = windows;
    curtain |= s[0].sun && s[0].t < tc - 4;
    curtain |= !s[0].sun && s[0].t > tc + 4;
    curtain &= !(state.s === 's' || state.s === 'sx');
    curtain |= state.s === 's' && state.p > 510;
    const res = {
      fan,
      windows,
      curtain,
      register,
      ac,
      acFan,
    };
    if (process.env.DEBUG) {
      console.log(
        'f012:', f012,
        't0:', s[0].t,
        'tt:', tt,
        'tc:', tc,
        'res:', res,
      );
    }
    return res;
  },
};
