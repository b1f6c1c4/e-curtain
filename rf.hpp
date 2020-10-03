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
    int _chipfd, _hfd;
    debouncer<4> _deb;
    std::thread _th;

    void thread_entry();
};
