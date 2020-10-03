#include "include/common.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include "io/si7021.hpp"
#include "dsp/filter.hpp"
#include "net/udp_client.hpp"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: e_curtain_cxx <host> <port> <tag>";
        return 1;
    }

    si7021 sensor{ "/dev/i2c-1" };
    std::array<lp_filter, 2> lps;
    udp_client<3> udp{ argv[1], std::atoi(argv[2]) };
    auto tag = std::atoi(argv[3]);

    using namespace std::chrono_literals;
    auto dt = 200ms; // 5Hz
    auto clk{ std::chrono::system_clock::now() };

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        clk += dt;

        arr_t<2> tr;
        sensor | lps;
        lps >> tr;

        udp << std::array{ static_cast<double>(tag), tr[0], tr[1] };

        auto aft{ std::chrono::system_clock::now() };
        auto diff{ clk - aft };
        if (diff > 1.1 * dt || diff < 0.9 * dt) { // abnormal skew, reset
            std::cerr << "Warning: abnormal clock skew" << std::endl;
            std::this_thread::sleep_for(dt);
            clk = std::chrono::system_clock::now();
        } else {
            std::this_thread::sleep_for(diff);
        }
    }
#pragma clang diagnostic pop
}
