#include "common.h"
#include "si7021.hpp"
#include "filter.hpp"
#include "udp_client.hpp"

int main() {
    si7021 sensor{ "/dev/i2c-1" };
    std::array<lp_filter, 2> lps;
    std::array<df_filter, 2> dfs;
    udp_client<4> udp{ "192.168.1.66", 23333 };
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        arr_t<2> tr0, tr, dtr;
        sensor >> tr0;

        tr0 >> lps >> tr;
        tr0 >> dfs >> dtr;

        udp << std::array{ tr[0], tr[1], dtr[0], dtr[1] };
    }
#pragma clang diagnostic pop
}
