#pragma once

#include <array>

using size_t = std::size_t;

template <size_t N>
using arr_t = std::array<double, N>;

template <size_t N>
struct source {
    virtual ~source() = default;
    virtual source &operator>>(arr_t<N> &r) = 0;
};

template <size_t N>
struct sink {
    virtual ~sink() = default;
    virtual sink &operator<<(const arr_t<N> &r) = 0;
};

template <size_t N>
struct sink_source : public source<N>, public sink<N> { };

template <size_t N, typename Tb>
decltype(auto) operator>>(source<N> &a, Tb &b) {
    arr_t<N> v;
    a >> v;
    b << v;
    return b;
}
