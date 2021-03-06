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

#define PORT 33706

template <size_t N>
struct arr_t : public std::array<double, N> {
    arr_t() : std::array<double, N>{ []() constexpr {
        std::array<double, N> r{};
        for (auto &v : r)
            v = INV;
        return r;
    }() } { }

    explicit arr_t(double v0) {
        for (auto &v : *this)
            v = v0;
    }

    template <typename T>
    arr_t(const std::array<T, N> &o) : std::array<double, N>{ [&o]() constexpr {
        std::array<double, N> r{};
        for (size_t i{ 0 }; i < N; i++)
            r[i] = o[i];
        return r;
    }() } { }

    template <typename ... TArgs>
    explicit arr_t(TArgs &&... l) : std::array<double, N>{ std::forward<TArgs>(l)... } { }
};

template <size_t N>
constexpr auto operator==(const arr_t<N> &l, const arr_t<N> &r) {
    return std::equal(l.begin(), l.end(), r.begin());
}

template <size_t N>
decltype(auto) operator<<(std::ostream &os, const arr_t<N> &r) {
    os << "[";
    for (size_t i{ 0 }; i < N; i++) {
        os << r[i];
        if (i < N - 1)
            os << ", ";
    }
    os << "]";
    return os;
}

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
struct sink_source : public source<N>, public sink<N> {
};

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
        s[i] << std::array{ v[i] };
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
    for (size_t i{ 0 }; i < N; i++)
        b[i] << arr_t<1>{ v[i] };
    return b;
}
