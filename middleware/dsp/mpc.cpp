#include "dsp/mpc.hpp"

mpc::mpc() : _u{}, _state_data{} { }

sink<7> &mpc::operator<<(const arr_t<7> &r) {
    struct4_T rr{};
    rr.signals.md = r[0]; // t0d
    rr.signals.ym[0] = r[1]; // t1
    rr.signals.ym[1] = r[2]; // t2
    rr.limits.umax[0] = +2;
    rr.limits.umax[1] = +2;
    rr.limits.umax[3] = r[3] ? 0 : 2; // f012
    rr.signals.ref[0] = r[4]; // tp1
    rr.signals.ref[1] = r[5]; // tp2
    rr.weights.y[0] = 1 - r[6]; // 1 - k
    rr.weights.y[1] = r[6]; // k
    struct9_T info;
    mpcmoveCodeGeneration(&_state_data, &rr, _u, &info);
    return *this;
}

source<3> &mpc::operator>>(arr_t<3> &r) {
    r[0] = _u[0];
    r[1] = _u[1];
    r[2] = _u[2];
    return *this;
}


