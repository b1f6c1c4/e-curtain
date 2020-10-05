#pragma once

#include "common.hpp"
#include <functional>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <stdexcept>
#include <cstring>
#include "dsp/debouncer.hpp"

template <size_t NI, size_t NO>
struct gpio : public sink<NO>, public source<NI> {
    gpio(const std::string &dev, const std::array<int, NI> &pi, const std::array<int, NO> &po,
         const std::array<bool, NO> &slow)
            : _chipfd{ open(dev.c_str(), 0) }, _ifd{ -1 }, _ofd{ -1 }, _slow{ slow } {
        if (_chipfd < 0)
            throw std::runtime_error("Cannot open gpio dev");

        gpiohandle_request req{};
        std::copy(pi.begin(), pi.end(), req.lineoffsets);
        req.flags = GPIOHANDLE_REQUEST_INPUT;
        req.lines = NI;
        std::strcpy(req.consumer_label, "e-curtain-cxx");
        if (ioctl(_chipfd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0)
            throw std::runtime_error("Cannot set gpio for read");
        _ifd = req.fd;

        std::copy(po.begin(), po.end(), req.lineoffsets);
        req.flags = GPIOHANDLE_REQUEST_OUTPUT;
        req.lines = NO;
        std::strcpy(req.consumer_label, "e-curtain-cxx");
        if (ioctl(_chipfd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0)
            throw std::runtime_error("Cannot set gpio for write");
        _ofd = req.fd;

        _th = std::thread{ &gpio::thread_entry, this };
    }

    virtual ~gpio() {
        if (_ifd >= 0)
            close(_ifd);
        if (_ofd >= 0)
            close(_ofd);
        if (_chipfd >= 0)
            close(_chipfd);
    }

    sink<NO> &operator<<(const arr_t<NO> &r) {
        using namespace std::chrono_literals;
        constexpr auto min{ 0.55ms };
        constexpr auto max{ 2.41ms };
        constexpr auto period{ 3ms };

        gpiohandle_data d{};
        std::fill(d.values, d.values + NO, 0);
        for (size_t i{ 0 }; i < NO; i++) {
            auto duration{ _slow[i] ? 0.85s : 0.25s };
            auto incr{ duration / period * 0.3 };
            auto t0{ std::chrono::steady_clock::now() + duration };
            for (size_t j{ 0 };; j++) {
                double v;
                if (IS_INV(_prev[i]) || j > incr)
                    v = r[i];
                else
                    v = j / incr * (r[i] - _prev[i]) + _prev[i];
                std::cerr << i << " " << j << " " << v << std::endl;
                auto dur{ min + (max - min) * v / 180 };
                d.values[i] = 1;
                if (ioctl(_ofd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d) < 0)
                    throw std::runtime_error("Cannot write gpio");
                std::this_thread::sleep_for(dur);
                d.values[i] = 0;
                if (ioctl(_ofd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d) < 0)
                    throw std::runtime_error("Cannot write gpio");
                std::this_thread::sleep_for(period - dur);
                if (t0 <= std::chrono::steady_clock::now())
                    break;
            }
        }
        _prev = r;
        return *this;
    }

    source<NI> &operator>>(arr_t<NI> &r) {
        _deb >> r;
        return *this;
    }

private:
    int _chipfd, _ifd, _ofd;
    debouncer<NI> _deb;
    arr_t<NO> _prev;
    std::array<bool, NO> _slow;
    std::thread _th;

    void thread_entry() {
        using namespace std::chrono_literals;
        gpiohandle_data d{};
        while (true) {
            if (ioctl(_ifd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &d) < 0)
                throw std::runtime_error("Cannot read gpio");
            arr_t<4> v;
            v[0] = d.values[0];
            v[1] = d.values[1];
            v[2] = d.values[2];
            v[3] = d.values[3];
            _deb << v;
            std::this_thread::sleep_for(8ms);
        }
    }
};
