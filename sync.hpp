#pragma once

#include "common.hpp"
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include <iostream>
#include <mutex>
#include <utility>

template <size_t N>
struct synchronizer : public sink_source<N> {
    template <typename Tt>
    explicit synchronizer(std::string name, Tt &&dt)
            : _name{std::move(name)}, _dt{ dt }, _v{ arr_t<N>{} }, _mtx{}, _clk{}, _callback{}, _thread(&synchronizer::thread_entry, this) { }

    template <typename Tt, typename Tf>
    synchronizer(std::string name, Tt &&dt, const Tf &cb)
            : _name{std::move(name)}, _dt{ dt }, _v{ arr_t<N>{} }, _mtx{}, _clk{}, _callback{ cb }, _thread(&synchronizer::thread_entry, this) { }

    virtual ~synchronizer() {
        _thread.join();
    }

    source<N> &operator>>(arr_t<N> &r) override {
        r = _v;
        return *this;
    }

    sink<N> &operator<<(const arr_t<N> &r) override {
        _v = r;
        return *this;
    }

    template <typename Tf>
    void set_callback(const Tf &cb) {
        std::lock_guard l{ _mtx };
        _callback = cb;
    }

    void immediate() {
        std::lock_guard l{ _mtx };
        _clk = std::chrono::system_clock::now();
        _clk += _dt;
        if (_callback)
            _callback();
    }

private:
    std::string _name;
    std::chrono::nanoseconds _dt;
    std::atomic<arr_t<N>> _v;
    std::mutex _mtx;
    std::chrono::system_clock::time_point _clk;
    std::function<void()> _callback;
    std::thread _thread;
    void thread_entry() {
        _clk = std::chrono::system_clock::now();

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
        while (true) {
            auto flag{ true };
            {
                std::lock_guard l{_mtx};
                auto curr{ std::chrono::system_clock::now() };
                if (_dt.count() != 0 && curr - _clk > 0.2 * _dt) {
                    std::cerr << now{} << " " << _name << " Warning: abnormal duration skew, skip" << std::endl;
                    _clk += _dt;
                    flag = false;
                } else {
                    _clk += _dt;
                    if (_callback)
                        _callback();
                }
            }

            if (_dt.count() == 0) {
                using namespace std::chrono_literals;
                if (!_callback)
                    std::this_thread::sleep_for(1s);
                continue;
            }

            auto aft{ std::chrono::system_clock::now() };
            auto diff{ _clk - aft };
            if (flag && (diff > 1.1 * _dt || diff < 0.9 * _dt)) {
                std::cerr << now{} << " " << _name << " Warning: abnormal clock skew, reset" << std::endl;
                std::this_thread::sleep_for(_dt);
                _clk = std::chrono::system_clock::now();
            } else {
                std::this_thread::sleep_for(diff);
            }
        }
#pragma clang diagnostic pop
    }
};
