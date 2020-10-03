#pragma once

#include "common.hpp"
extern "C" {
#include "../../libdumbac/mpcmoveCodeGeneration.h"
}

struct mpc : public sink<7>, public source<3> {
    mpc();
    sink<7> &operator<<(const arr_t<7> &r) override;
    source<3> &operator>>(arr_t<3> &r) override;

private:
    struct3_T _state_data;
    double _u[3];
};
