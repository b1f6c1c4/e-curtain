#include "mpc.hpp"

mpc::mpc() : silence(false), k(2.0 / 3), _u{}, _state_data{} { }

sink<5> &mpc::operator<<(const arr_t<5> &r) {
    struct4_T rr{};
    rr.signals.ref[0] = r[0];
    rr.signals.ref[1] = r[1];
    rr.signals.md = r[2];
    rr.signals.ym[0] = r[3];
    rr.signals.ym[1] = r[4];
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


