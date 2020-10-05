#pragma once

#include "common.hpp"
#include "io/external.hpp"
#include "io/shared.hpp"

struct buzzer : public py {
    buzzer();

    void on();
    void off();

private:
    std::unique_ptr<lock_shared<pwm_chan>> _l;
};
