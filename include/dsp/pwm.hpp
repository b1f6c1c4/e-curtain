#pragma once

#include "common.hpp"

struct pwm : public sink_source<1> {
    pwm(size_t max);
    sink<1> &operator<<(const arr_t<1> &r) override;
    source<1> &operator>>(arr_t<1> &r) override;
private:
    size_t _cnt, _max;
    double _v, _prev;
};
