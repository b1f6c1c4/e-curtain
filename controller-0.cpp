#include "common.hpp"
#include <iostream>
#include <chrono>
#include <limits>
#include "udp_server.hpp"
#include "filter.hpp"
#include "mpc.hpp"

int main() {
    udp_server<5> udp{ 23333 };

    auto t0{ std::numeric_limits<double>::quiet_NaN() };
    auto h0{ std::numeric_limits<double>::quiet_NaN() };
    auto t1{ std::numeric_limits<double>::quiet_NaN() };
    auto h1{ std::numeric_limits<double>::quiet_NaN() };
    auto t2{ std::numeric_limits<double>::quiet_NaN() };
    auto h2{ std::numeric_limits<double>::quiet_NaN() };
    auto f012{ std::numeric_limits<double>::quiet_NaN() };
    auto f12{ std::numeric_limits<double>::quiet_NaN() };
    auto ac{ std::numeric_limits<double>::quiet_NaN() };

    auto clk0{ std::chrono::steady_clock::now() };

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        {
            arr_t<5> d;
            udp >> d;
            switch (static_cast<int>(d[0])) {
                case 0:
                    t0 = d[1];
                    h0 = d[2];
                    break;
                case 1:
                    t1 = d[1];
                    h1 = d[2];
                    break;
                case 2:
                    t2 = d[1];
                    h2 = d[2];
                    break;
                case 3:
                    f012 = d[1];
                    f12 = d[2];
                    ac = d[3];
                    break;
                default:
                    continue;
            }
            arr_t<10> log;
            log[0] = (std::chrono::steady_clock::now() - clk0).count();
            log[1] = t0;
            log[2] = h0;
            log[3] = f012;
            log[4] = f12;
            log[5] = ac;
            log[6] = t1;
            log[7] = t2;
            log[8] = h1;
            log[9] = h2;
            std::cout.write(reinterpret_cast<const char *>(log.data()), sizeof(double) * log.size());
            std::cout.flush();
        }
    }
#pragma clang diagnostic pop
}
