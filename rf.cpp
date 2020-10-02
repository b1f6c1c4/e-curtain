#include "rf.hpp"
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <stdexcept>

rf::gpio::gpio(int pin)
        : _pin{ pin }, _fd{ -1 }, _chipfd{ open("/dev/gpiochip0", O_RDWR | O_CLOEXEC) }, _v{ false } {
    if (_chipfd < 0)
        throw std::runtime_error("Cannot open gpio dev");

    {
        std::ofstream os("/sys/class/gpio/export", std::ios::out);
        os << _pin << std::endl;
    }
    auto path{ "/sys/class/gpio/gpio" + std::to_string(_pin) };
    {
        std::ofstream os(path + "/direction", std::ios::out);
        os << "in" << std::endl;
    }

    gpioevent_request req{};
    req.eventflags = GPIOEVENT_REQUEST_BOTH_EDGES;
    req.lineoffset = _pin;
    {
        std::string label("e-curtain-cxx");
        std::copy(label.begin(), label.end(), req.consumer_label);
    }
    req.handleflags = GPIOHANDLE_REQUEST_INPUT;
    if (ioctl(_chipfd, GPIO_GET_LINEEVENT_IOCTL, &req) < 0)
        throw std::runtime_error("Cannot set gpio");
    if ((_fd = open((path + "/value").c_str(), O_RDONLY | O_NONBLOCK) < 0))
        throw std::runtime_error("Cannot open gpio for polling");
}

rf::gpio::~gpio() {
    if (_fd >= 0)
        close(_fd);
    if (_chipfd >= 0)
        close(_chipfd);
    try {
        std::ofstream os("/sys/class/gpio/unexport", std::ios::out);
        os << _pin;
    } catch (...) {
        // ignored
    }
}

rf::rf() : _gpios{ 29, 28, 27, 26 }, _th{ &rf::thread_entry, this } { }

source<4> &rf::operator>>(arr_t<4> &r) {
    _deb >> r;
    return *this;
}

void rf::thread_entry() {
    auto epfd{ epoll_create1(0) };
    if (epfd < 0)
        throw std::runtime_error("Cannot epoll_create1");

    {
        epoll_event ev{};
        ev.events = EPOLLET;
        for (auto &g : _gpios) {
            ev.data.fd = g._fd;
            ev.data.ptr = reinterpret_cast<void *>(&g);
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, g._fd, &ev) < 0)
                throw std::runtime_error("Cannot epoll_ctl");
        }
    }

    while (true) {
        epoll_event ev;
        auto n{ epoll_wait(epfd, &ev, 1, -1) };
        if (n < 0)
            throw std::runtime_error("Cannot epoll_wait");
        gpiohandle_data d;
        if (ioctl(ev.data.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &d) < 0)
            throw std::runtime_error("Cannot read after epoll");
        auto g{ reinterpret_cast<gpio *>(ev.data.ptr) };
        g->_v = d.values[0];
        _deb << _gpios;
    }
}
