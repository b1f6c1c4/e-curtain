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

ir::ir(const std::string &dev)
        : _chipfd{ open(dev.c_str(), 0) }, _ofd{ -1 } {
    if (_chipfd < 0)
        throw std::runtime_error("Cannot open gpio dev");
    g_make_realtime();

    gpiohandle_request req{};
    req.lineoffsets[0] = 24;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.lines = 1;
    std::strcpy(req.consumer_label, "e-curtain-cxx");
    if (ioctl(_chipfd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0)
        throw std::runtime_error("Cannot set gpio for write");
    _ofd = req.fd;
}

sink<12> &ir::operator<<(const arr_t<12> &r) {
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
    return *this;
}
