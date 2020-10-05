#include "common.hpp"
#include "io/si7021.hpp"
#include "dsp/filter.hpp"
#include "net/udp_client.hpp"
#include "sync.hpp"

#ifndef TAG
#error "Macro TAG not defined"
#endif

using namespace std::chrono_literals;

int main(int argc, char *argv[]) {
    std::string host{ "controller-2" };
    if (argc == 1) {
        // use default host
    } else if (argc == 2) {
        host = argv[1];
    } else {
        std::cerr << "Usage: ./H[12] [<host>]" << std::endl;
        std::cerr << "Note: The default <host> is controller-2" << std::endl;
        return 1;
    }

    si7021 i_sensor{ "/dev/i2c-1" };
    std::array<lp_filter, 2> lps;
    udp_client<3> i_udp_client{ host, PORT };
    synchronizer<0> s_sensor{ "s_sensor", 200ms, [&]() {
        arr_t<2> tr;
        i_sensor | lps;
        lps >> tr;
        i_udp_client << std::array{ static_cast<double>(TAG), tr[0], tr[1] };
    }};
}
