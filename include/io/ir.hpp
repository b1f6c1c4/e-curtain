#pragma once

#include "common.hpp"

struct ir : public sink<12> {
    explicit ir(const std::string &dev);

    sink<12> &operator<<(const arr_t<12> &r);

private:
    int _chipfd, _ofd;
};
