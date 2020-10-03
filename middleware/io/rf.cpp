#include "io/rf.hpp"
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <stdexcept>
#include <cstring>

rf::rf(const std::string &dev) : _chipfd{ open(dev.c_str(), 0) }, _hfd{ -1 } {
    if (_chipfd < 0)
        throw std::runtime_error("Cannot open gpio dev");

    gpiohandle_request req{};
    req.lineoffsets[0] = 12;
    req.lineoffsets[1] = 16;
    req.lineoffsets[2] = 20;
    req.lineoffsets[3] = 21;
    req.flags = GPIOHANDLE_REQUEST_INPUT;
    req.lines = 4;
    std::strcpy(req.consumer_label, "e-curtain-cxx");
    if (ioctl(_chipfd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0)
        throw std::runtime_error("Cannot set gpio");
    _hfd = req.fd;

    _th = std::thread{ &rf::thread_entry, this };
}

rf::~rf() {
    if (_hfd >= 0)
        close(_hfd);
    if (_chipfd >= 0)
        close(_chipfd);
}

source<4> &rf::operator>>(arr_t<4> &r) {
    _deb >> r;
    return *this;
}

void rf::thread_entry() {
    using namespace std::chrono_literals;
    gpiohandle_data d{};
    while (true) {
        if (ioctl(_hfd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &d) < 0)
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
