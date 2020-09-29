#pragma once

#include <cstddef>
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

template <size_t N, typename T>
decltype(auto) operator>>(std::array<T, N> &s, arr_t<N> &v) {
    arr_t<1> buf;
    for (size_t i{ 0 }; i < N; i++) {
        s[i] >> buf;
        v[i] = buf[0];
    }
    return s;
}

template <size_t N, typename T>
decltype(auto) operator<<(std::array<T, N> &s, const arr_t<N> &v) {
    for (size_t i{ 0 }; i < N; i++)
        s[i] << std::array{v[i]};
    return s;
}

template <typename T1, typename T2>
decltype(auto) operator>>(T1 &&l, T2 &&r) {
    return std::forward<T2>(r) << std::forward<T1>(l);
}
