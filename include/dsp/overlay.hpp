#pragma once

#include "common.hpp"

template <size_t N>
struct overlay : public sink_source<N> {
    sink<N> &operator<<(const arr_t<N> &r) override {
        for (size_t i{ 0 }; i < N; i++) {
            if (IS_INV(_v[i]) || _v[i] != r[i])
                _d[i] = r[i];
            else
                _d[i] = INV;
        }
        return *this;
    }

    source<N> &operator>>(arr_t<N> &r) override {
        r = _d;
        for (size_t i{ 0 }; i < N; i++)
            if (!IS_INV(_d[i]))
                _v[i] = _d[i];
        _d = arr_t<N>{};
        return *this;
    }
private:
    arr_t<N> _v, _d;
};