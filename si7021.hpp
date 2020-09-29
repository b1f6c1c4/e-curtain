#pragma once

#include "common.h"
#include <string>

struct si7021 : public source<2> {
    si7021(const std::string &dev);
    ~si7021();
    source &operator>>(arr_t<2> &r) override;

private:
    int _fd;
};
