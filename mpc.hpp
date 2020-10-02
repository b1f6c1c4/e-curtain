#pragma once

#include "common.hpp"
#include "mpcmoveCodeGeneration.h"

struct mpc : public sink<5>, public source<3> {
    bool silence;
    double k;

    mpc();
    sink<5> &operator<<(const arr_t<5> &r) override;
    source<3> &operator>>(arr_t<3> &r) override;

private:
    struct3_T _state_data;
    double _u[3];
};
