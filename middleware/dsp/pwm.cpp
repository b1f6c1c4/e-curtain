#include "dsp/pwm.hpp"

pwm::pwm(size_t max) : _cnt{ 0 }, _max{ max }, _v{ 0 }, _prev{ INV } { }

sink<1> &pwm::operator<<(const arr_t<1> &r) {
    _v = r[0];
    return *this;
}

source<1> &pwm::operator>>(arr_t<1> &r) {
    if (_cnt < _max * _v && _prev != 1)
        r[0] = _prev = 1;
    else if (_cnt >= _max * _v && _prev != 0)
        r[0] = _prev = 0;
    else
        r[0] = INV;
    _cnt = (_cnt + 1) % _max;
    return *this;
}
