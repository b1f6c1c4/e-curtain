#pragma once

#include "common.hpp"

struct buzzer {
    buzzer();
    virtual ~buzzer();

    void on();
    void off();

private:
    void write(const std::string &str);
    FILE *_fp;
};