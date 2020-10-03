#pragma once

#include "common.hpp"
#include <functional>
#include <thread>
#include "debouncer.hpp"

struct rf : public source<4> {
    rf(const std::string &dev);
    virtual ~rf();
    source<4> &operator>>(arr_t<4> &r);

private:
    struct gpio {
        gpio(int pin);
        virtual ~gpio();
        operator double() const { return _v; }
        void init(int chipfd);
        int _pin, _fd;
        bool _v;
    };

    int _fd;
    std::array<gpio, 4> _gpios;
    debouncer<4> _deb;
    std::thread _th;

    void thread_entry();
};
