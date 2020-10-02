#pragma once

#include "common.hpp"
#include <chrono>
#include <mutex>
#include <condition_variable>

template <size_t N>
struct debouncer : public sink_source<N> {
    debouncer() : _state{ IDLE } { }

    sink<N> &operator<<(const arr_t<N> &r) override {
        auto now{ clk::now() };
        switch (_state) {
            case IDLE:
                if (r != zero) {
                    _state = STAGE1;
                    _c1 = now; // enter stage1
                    _stage1 = r;
                }
                break;
            case STAGE1:
                if (now < _c1 + _margin1) { // fast transform
                    if (r != _stage1) {
                        _c1 = now; // restart stage1
                        _stage1 = r;
                    }
                } else { // slow transform
                    _state = STAGE2;
                    _stage2 = _stage1; // confirm the old one
                    if (r != _stage1) {
                        _state = STAGE21;
                        _c1 = now; // the new one enters stage1
                    }
                }
                break;
            case STAGE2:
                if (r != _stage2) {
                    _state = STAGE21;
                    _c1 = now; // the new one enters stage1
                    _stage1 = r;
                }
                break;
            case STAGE21:
                if (now < _c1 + _margin1) { // fast transform
                    if (r != _stage1) {
                        _c1 = now; // restart stage1
                        _stage1 = r;
                    }
                } else { // slow transform
                    if (r == zero) { // this is actually a release
                        _state = IDLE;
                        std::lock_guard l{ _mtx3 };
                        _stage3 = _stage2;
                        _c3 = now; // start stage3
                        _cv3.notify_one();
                    } else {
                        _state = STAGE2;
                        // confirm the old one by merging
                        for (size_t i{0}; i < N; i++)
                            if (_stage1[i] != 0.0)
                                _stage2[i] = 1.0;
                        if (r != _stage1) {
                            _state = STAGE21;
                            _c1 = now; // the new one enters stage1
                        }
                    }
                }
                break;
        }
        return *this;
    }

    source<N> &operator>>(arr_t<N> &r) override {
        std::unique_lock l{ _mtx3 };
        while (_stage3 == zero || clk::now() >= _c3 + _margin3)
            _cv3.wait(l);
        r = _stage3;
        return *this;
    }

private:
    typedef std::chrono::steady_clock clk;
    static const arr_t<N> zero;

    enum {
        // nothing pressed at all (may have confirmed press + release)
        IDLE,
        // only brief press
        STAGE1,
        // only confirmed press
        STAGE2,
        // confirmed press + brief [press|release]
        STAGE21,
    } _state;

    // The brief [press|release], managed by operator<<
    arr_t<N> _stage1;
    clk::time_point _c1; // stage1 entrance time

    // Time to confirm a brief [press|release]
    static constexpr auto _margin1 = []() constexpr {
        using namespace std::chrono_literals;
        return 30ms;
    }();

    // The confirmed press (aggregated), managed by operator<<
    arr_t<N> _stage2;

    // The confirmed press + release, managed by operator>>
    arr_t<N> _stage3;
    clk::time_point _c3; // stage3 entrance time
    // Locking mechanism for waiting for _stage3 to be ready
    std::mutex _mtx3;
    std::condition_variable _cv3;

    // Time to disposed a confirmed press + release
    static constexpr auto _margin3 = []() constexpr {
        using namespace std::chrono_literals;
        return 1s;
    }();
};

template <size_t N>
const arr_t<N> debouncer<N>::zero{ 0.0 };
