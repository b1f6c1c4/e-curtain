#include "include/common.hpp"
#include <chrono>
#include <thread>
#include "io/si7021.hpp"
#include "dsp/filter.hpp"
#include "net/udp_client.hpp"
#include "sync.hpp"

int main(int argc, char *argv[]) {
    si7021 sensor{ "/dev/i2c-1" };
    std::array<lp_filter, 2> lps;
    udp_client<3> udp{ "controller-0", PORT };
    auto tag{ 2 };

    using namespace std::chrono_literals;
    synchronizer<0> sen_s{"sen_s", 200ms, [&](){
        arr_t<2> tr;
        sensor | lps;
        lps >> tr;
        udp << std::array{ static_cast<double>(tag), tr[0], tr[1] };
    }};
}
