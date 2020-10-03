#include "rf.hpp"
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <stdexcept>
#include <cstring>

rf::rf(const std::string &dev) : _fd{ open(dev.c_str(), 0) }, _gpios{ 12, 16, 20, 21 } {
    if (_fd < 0)
        throw std::runtime_error("Cannot open gpio dev");

    for (auto &g: _gpios)
        g.init(_fd);

    _th = std::thread{ &rf::thread_entry, this };
}

rf::~rf() {
    if (_fd >= 0)
        close(_fd);
}

rf::gpio::gpio(int pin) : _pin{ pin }, _fd{ -1 } { }

void rf::gpio::init(int chipfd) {
    gpioevent_request req{};
    req.lineoffset = _pin;
    req.handleflags = GPIOHANDLE_REQUEST_INPUT;
    req.eventflags = GPIOEVENT_REQUEST_BOTH_EDGES;
    std::strcpy(req.consumer_label, "e-curtain-cxx");
    if (ioctl(chipfd, GPIO_GET_LINEEVENT_IOCTL, &req) < 0)
        throw std::runtime_error("Cannot set gpio");
    _fd = req.fd;

    gpiohandle_data d{};
    if (ioctl(_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &d) < 0)
        throw std::runtime_error("Cannot get gpio initial value");
    _v = d.values[0];
}

rf::gpio::~gpio() {
    if (_fd >= 0)
        close(_fd);
}

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
        ev.events = EPOLLIN | EPOLLPRI;
        for (auto &g : _gpios) {
            ev.data.ptr = reinterpret_cast<void *>(&g);
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, g._fd, &ev) < 0)
                throw std::runtime_error("Cannot epoll_ctl");
        }
    }

    _deb << _gpios;
    while (true) {
        epoll_event ev;
        auto n{ epoll_wait(epfd, &ev, 1, -1) };
        if (n < 0)
            throw std::runtime_error("Cannot epoll_wait");
        auto g{ reinterpret_cast<gpio *>(ev.data.ptr) };
        gpiohandle_data d;
        if (ioctl(g->_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &d) < 0)
            throw std::runtime_error("Cannot read after epoll");
        g->_v = d.values[0];
        _deb << _gpios;
    }
}
