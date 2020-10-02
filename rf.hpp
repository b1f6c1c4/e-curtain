#pragma once

#include "common.hpp"
#include <functional>
#include <thread>
#include "debouncer.hpp"

struct rf : public source<4> {
    rf();
    source<4> &operator>>(arr_t<4> &r);

private:
    struct gpio {
        gpio(int pin);
        virtual ~gpio();
        friend class rf;
        operator double() const { return _v; }
    private:
        int _pin, _fd, _chipfd;
        bool _v;
    };

    std::array<gpio, 4> _gpios;
    debouncer<4> _deb;
    std::thread _th;

    void thread_entry();
};
