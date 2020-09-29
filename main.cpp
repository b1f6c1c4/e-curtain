#include "common.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "si7021.hpp"
#include "filter.hpp"
#include "udp_client.hpp"

int main() {
    si7021 sensor{ "/dev/i2c-1" };
    std::array<lp_filter, 2> lps;
    std::array<df_filter, 2> dfs;
    udp_client<4> udp{ "192.168.1.66", 23333 };

    using namespace std::chrono_literals;
    auto dt = 500ms; // 2Hz
    auto clk{ std::chrono::system_clock::now() };

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        clk += dt;

        arr_t<2> tr0, tr, dtr;
        sensor >> tr0;

        tr0 >> lps >> tr;
        tr0 >> dfs >> dtr;

        udp << std::array{ tr[0], tr[1], dtr[0], dtr[1] };

        auto aft{ std::chrono::system_clock::now() };
        auto diff{ clk - aft };
        if (diff > 1.05 * dt || diff < 0.95 * dt) { // abnormal skew, reset
            clk = aft;
            std::cerr << "Warning: abnormal clock skew" << std::endl;
            std::this_thread::sleep_for(dt);
        } else {
            std::this_thread::sleep_for(diff);
        }
    }
#pragma clang diagnostic pop
}
