#pragma once

#include "common.hpp"
#include <mutex>
#include <condition_variable>

template <size_t M>
struct lock_shared;

template <size_t M>
struct shared {
    friend class lock_shared<M>;
private:
    size_t _v{ M };
    std::mutex _mtx;
    std::condition_variable _cv;
};

template <size_t M>
struct lock_shared final {
    explicit lock_shared(shared<M> &s) : _s{ s } {
        std::unique_lock l{ _s._mtx };
        while (_s._v == 0)
            _s._cv.wait(l);
        _s._v--;
    }
    ~lock_shared() {
        std::unique_lock l{ _s._mtx };
        _s._v++;
        _s._cv.notify_one();
    }
private:
    shared<M> &_s;
};

constexpr const size_t pwm_chan{ 1 };
extern shared<pwm_chan> g_pwm;
