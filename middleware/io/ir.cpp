#include "io/ir.hpp"
#include "common.hpp"
#include <functional>
#include <thread>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <stdexcept>
#include <cstring>
#include "io/realtime.hpp"

static arr_t<12> g_power{ 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
static arr_t<12> g_level{ 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

ir::ir(const std::string &dev, int port_tx, int port_chk) : _chipfd{ open(dev.c_str(), 0) }, _ifd{ -1 }, _ofd{ -1 }, _state{ -1 }, _cycle{ 0 } {
    if (_chipfd < 0)
        throw std::runtime_error("Cannot open gpio dev for ir");
    g_make_realtime();

    gpiohandle_request req{};
    req.lineoffsets[0] = port_chk;
    req.flags = GPIOHANDLE_REQUEST_INPUT;
    req.lines = 1;
    std::strcpy(req.consumer_label, "e-curtain-cxx");
    if (ioctl(_chipfd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0)
        throw std::runtime_error("Cannot set gpio for ir read");
    _ifd = req.fd;

    req.lineoffsets[0] = port_tx;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.lines = 1;
    std::strcpy(req.consumer_label, "e-curtain-cxx");
    if (ioctl(_chipfd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0)
        throw std::runtime_error("Cannot set gpio for ir write");
    _ofd = req.fd;
}

sink<1> &ir::operator<<(const arr_t<1> &r) {
    using namespace std::chrono_literals;
    auto next{ static_cast<int>(r[0]) };
    for (size_t i{ 0 }; i < 3; i++) {
        if (i)
            std::this_thread::sleep_for(1s);
        if (_state < 0) {
            if (!check())
                _state = 0;
            else if (power()) {
                std::cout << "Warning: Cannot turn off IR (" << i + 1 << "/3)" << std::endl;
                _state = -1;
                continue;
            }
        } else if (_state == 0) {
            if (check()) {
                std::cout << "Warning: Unexpected IR turning on (" << i + 1 << "/3)" << std::endl;
                _state = -1;
                continue;
            } else if (++_cycle >= max_cycles) {
                if (power())
                    std::cout << "Info: Automatic IR re-cycle succeeded" << std::endl;
                else {
                    std::cout << "Warning: Cannot turn off IR (" << i + 1 << "/3)" << std::endl;
                    _state = -1;
                    continue;
                }
            }
        } else if (!check()) {
            std::cout << "Warning: Unexpected IR turning off (" << i + 1 << "/3)" << std::endl;
            _state = 0;
        }
        if (_state == next)
            break;
        if (!next) {
            if (power()) {
                std::cout << "Warning: Cannot turn off IR (" << i + 1 << "/3)" << std::endl;
                _state = -1;
                continue;
            }
            break;
        }
        if (!_state) {
            if (!power()) {
                std::cout << "Warning: Cannot turn on IR (" << i + 1 << "/3)" << std::endl;
                _state = -1;
                continue;
            }
            _state = 3;
        }
        while (_state != next) {
            issue(g_level);
            std::this_thread::sleep_for(75ms);
            _state = (_state + 1) % 3 + 1;
        }
        if (!_state == !check())
            break;
    }
    if (_state != next)
        std::cout << "Warning: Exiting with async state: " << _state << std::endl;
    return *this;
}

bool ir::check() const {
    gpiohandle_data d{};
    if (ioctl(_ifd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &d) < 0)
        throw std::runtime_error("Cannot read gpio for ir");
    return d.values[0];
}

void ir::issue(const arr_t<12> &r) {
    using namespace std::chrono_literals;
    constexpr auto period{ 1700us };
    std::cout << "Sending IR: " << r << std::endl;

    gpiohandle_data d{};
    for (size_t i{ 0 }; i < r.size(); i++) {
        auto duration{ r[i] ? 1260us : 420us };
        d.values[0] = 1;
        if (ioctl(_ofd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d) < 0)
            throw std::runtime_error("Cannot write gpio");
        auto t0{ std::chrono::steady_clock::now() + period };
        std::this_thread::sleep_for(duration);
        d.values[0] = 0;
        if (ioctl(_ofd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d) < 0)
            throw std::runtime_error("Cannot write gpio");
        std::this_thread::sleep_for(t0 - std::chrono::steady_clock::now());
    }
}

bool ir::power() {
    using namespace std::chrono_literals;
    issue(g_power);
    std::this_thread::sleep_for(150ms);
    auto res{ check() };
    _cycle = 0;
    _state = res ? 3 : 0;
    return res;
}
