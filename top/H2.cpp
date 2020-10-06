#include "common.hpp"
#include "io/si7021.hpp"
#include "dsp/filter.hpp"
#include "net/udp_client.hpp"
#include "sync.hpp"

using namespace std::chrono_literals;

int main(int argc, char *argv[]) {
    std::string host;
    if (argc == 2) {
        host = argv[1];
    } else {
        std::cout << "Usage: ./H2 <host>" << std::endl;
        return 1;
    }

    si7021 i_sensor{ "/dev/i2c-1" };
    std::array<lp_filter, 2> lps;
    udp_client<3> i_udp_client{ host, PORT };
    synchronizer<0> s_sensor{ "s_sensor", 200ms, [&]() {
        arr_t<2> tr;
        i_sensor | lps;
        lps >> tr;
        i_udp_client << std::array{ 2.0, tr[0], tr[1] };
    } };
}
