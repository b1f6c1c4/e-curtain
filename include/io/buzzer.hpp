#pragma once

#include "common.hpp"
#include "io/external.hpp"

struct buzzer : public py {
    buzzer();

    void on();
    void off();
};