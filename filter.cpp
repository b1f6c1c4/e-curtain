#include "filter.hpp"

constexpr std::array<double, 31> df_b{
        1.469213114392,    1.487089555831,    1.481380035058,    1.453635226479,
        1.405435163037,     1.33838714122,    1.254123587526,    1.154299889342,
        1.040592193005,    0.914695172329,   0.7783197709371,   0.6331909211106,
        0.4810452430128,   0.3236287271023,   0.1626944032424,                -0,
        -0.1626944032424,  -0.3236287271023,  -0.4810452430128,  -0.6331909211106,
        -0.7783197709371,   -0.914695172329,   -1.040592193005,   -1.154299889342,
        -1.254123587526,    -1.33838714122,   -1.405435163037,   -1.453635226479,
        -1.481380035058,   -1.487089555831,   -1.469213114392
};

constexpr std::array<double, 31> lp_b{
        -0.02116143477456, 0.003713943557878, 0.006486822815972,  0.01104542652924,
        0.01725733772768,  0.02490351554676,  0.03375572555256,  0.04344681089898,
        0.05360234899511,  0.06376085615856,   0.0734386623807,  0.08219649473202,
        0.08952399322347,  0.09505942784682,  0.09852723001524,  0.09968872619232,
        0.09852723001524,  0.09505942784682,  0.08952399322347,  0.08219649473202,
        0.0734386623807,  0.06376085615856,  0.05360234899511,  0.04344681089898,
        0.03375572555256,  0.02490351554676,  0.01725733772768,  0.01104542652924,
        0.006486822815972, 0.003713943557878, -0.02116143477456
};

fir_filter::fir_filter() : _empty{ true }, _cursor{ 0 }, _circular{} { }

sink<1> &fir_filter::operator<<(const arr_t<1> &r) {
    if (_empty) {
        for (auto &v : _circular)
            v = r[0];
        _empty = false;
        _cursor++;
    } else {
        _circular[_cursor++] = r[0];
        _cursor %= _order + 1;
    }
    return *this;
}

source<1> &lp_filter::operator>>(arr_t<1> &r) {
    auto t{ 0.0 };
    for (size_t i{ 0 }; i <= _order; i++) {
        t += lp_b[i];
    }
    r[0] = 0;
    for (size_t i{ 0 }; i <= _order; i++) {
        r[0] += _circular[i] * lp_b[(_order + _cursor - i) % (_order + 1)];
    }
    r[0] /= t;
    return *this;
}

source<1> &df_filter::operator>>(arr_t<1> &r) {
    r[0] = 0;
    for (size_t i{ 0 }; i <= _order; i++) {
        r[0] += _circular[i] * df_b[(_order + _cursor - i) % (_order + 1)];
    }
    return *this;
}

