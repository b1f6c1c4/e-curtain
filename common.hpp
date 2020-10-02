#pragma once

#include <cstddef>
#include <array>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <limits>
#include <cmath>

using size_t = std::size_t;

#define INV std::numeric_limits<double>::quiet_NaN()
#define IS_INV(x) std::isnan((x))

template <size_t N>
struct arr_t : public std::array<double, N> {
    arr_t() : std::array<double, N>{[]() constexpr {
        std::array<double, N> r;
        for (auto &v : r)
            v = INV;
        return r;
    }()} { }

    template <typename ... TArgs>
    explicit arr_t(TArgs && ... l) : std::array<double, N>{std::forward<TArgs>(l)...} { }
};

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
decltype(auto) operator<<(std::array<T, N> &s, arr_t<N> &&v) {
    for (size_t i{ 0 }; i < N; i++)
        s[i] << std::array{v[i]};
    return s;
}

template <typename T, size_t N>
decltype(auto) operator|(source<N> &a, T &b) {
    arr_t<N> v;
    a >> v;
    b << v;
    return b;
}

template <typename T, size_t N>
decltype(auto) operator|(source<N> &a, std::array<T, N> &b) {
    arr_t<N> v;
    a >> v;
    b << v;
    return b;
}

struct now {
    friend inline decltype(auto) operator<<(std::ostream &os, now) {
        auto now{ std::chrono::system_clock::now() };
        auto itt{ std::chrono::system_clock::to_time_t(now) };
        os << std::put_time(gmtime(&itt), "%FT%TZ.");
        os << std::setfill('0') << std::setw(9) << (now.time_since_epoch().count() % 1000000000);
        return os;
    }
};
