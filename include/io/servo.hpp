#pragma once

#include <thread>
#include "common.hpp"
#include "io/external.hpp"

struct servo : public sink<1> {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    servo(int pin);
#pragma clang diagnostic pop
    sink<1> &operator<<(const arr_t<1> &r) override;

private:
    static constexpr const double freq{ 100 };
    int _pin;
};
