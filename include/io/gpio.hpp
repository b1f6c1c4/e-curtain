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
#include "io/realtime.hpp"

template <size_t NI, size_t NO>
struct gpio : public sink<NO>, public source<NI> {
    gpio(const std::string &dev, const std::array<int, NI> &pi, const std::array<int, NO> &po)
            : _chipfd{ open(dev.c_str(), 0) }, _ifd{ -1 }, _ofd{ -1 } {
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
        constexpr auto lax{ 250ms };

        gpiohandle_data d{};
        std::fill(d.values, d.values + NO, 0);
        for (size_t i{ 0 }; i < NO; i++) {
            for (size_t j{ 0 }; j < 1; j++) {
                double v;
                if (IS_INV(r[i]))
                    v = 0;
                else
                    v = r[i];
                auto dur{ min + (max - min) * v / 180 };
                d.values[i] = IS_INV(r[i]) ? 0 : 1;
                if (ioctl(_ofd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d) < 0)
                    throw std::runtime_error("Cannot write gpio");
                std::this_thread::sleep_for(dur);
                d.values[i] = 0;
                if (ioctl(_ofd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d) < 0)
                    throw std::runtime_error("Cannot write gpio");
                std::this_thread::sleep_for(period - dur);
            }
            if (!IS_INV(r[i]))
                std::this_thread::sleep_for(lax);
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
    std::thread _th;

    void thread_entry() {
        using namespace std::chrono_literals;
        g_make_realtime();

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
