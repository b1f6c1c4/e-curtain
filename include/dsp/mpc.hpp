#pragma once

#include "common.hpp"
#include "libdumbac.h"

struct mpc : public sink<15>, public source<7> {
    mpc();
    sink<15> &operator<<(const arr_t<15> &r) override;
    source<7> &operator>>(arr_t<7> &r) override;

private:
    libdumbacModelClass _controller;
};
