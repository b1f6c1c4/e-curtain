#include "filter.hpp"

constexpr std::array<double,13> df_b{
        0.02250692741948,  -0.0753941435364,  -0.0357923324939,   0.1191856355115,
        0.2817705525623,   0.2506500788006,                -0,  -0.2506500788006,
        -0.2817705525623,  -0.1191856355115,   0.0357923324939,   0.0753941435364,
        -0.02250692741948
};

constexpr std::array<double,13> lp_b{
        -0.008989547260139, -0.02329586182082, -0.01988336730288,  0.03476143199288,
        0.1454054014647,   0.2633200585847,   0.3145523633625,   0.2633200585847,
        0.1454054014647,  0.03476143199288, -0.01988336730288, -0.02329586182082,
        -0.008989547260139
};

fir_filter::fir_filter() : _empty{ true }, _cursor{ 0 }, _circular{} { }

sink<1> &fir_filter::operator<<(const arr_t<1> &r) {
    if (_empty) {
        for (auto &v : _circular)
            v = r[0];
        _empty = true;
        _cursor++;
    } else {
        _circular[_cursor++] = r[0];
        _cursor %= _order + 1;
    }
    return *this;
}

source<1> &lp_filter::operator>>(arr_t<1> &r) {
    r[0] = 0;
    for (size_t i{ 0 }; i <= _order; i++) {
        r[0] += _circular[i] * lp_b[(_order + _cursor - i) % (_order + 1)];
    }
    return *this;
}

source<1> &df_filter::operator>>(arr_t<1> &r) {
    r[0] = 0;
    for (size_t i{ 0 }; i <= _order; i++) {
        r[0] += _circular[i] * df_b[(_order + _cursor - i) % (_order + 1)];
    }
    return *this;
}
