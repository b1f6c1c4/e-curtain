#include "common.hpp"
#include "net/udp_client.hpp"
#include "net/udp_server.hpp"
#include "dsp/filter.hpp"
#include "io/rf.hpp"
#include "io/servo.hpp"
#include "sync.hpp"

using namespace std::chrono_literals;

int main() {
    rf rf_inst{ "/dev/gpiochip0" };

    std::array<servo, 4> p{ 10, 12, 8, 16 };

    udp_server<3> udp_s{ PORT };
    udp_client<6> udp_c{ "controller-2", PORT };

    synchronizer<4> rf_s{ "rf_s", 0s, [&]() {
        arr_t<4> v;
        rf_inst >> v;
        std::cout << "Key pressed: ";
        std::cout << v[0] << " ";
        std::cout << v[1] << " ";
        std::cout << v[2] << " ";
        std::cout << v[3] << std::endl;
        if (v[0])
            p << arr_t<4>{ 180.0, 180.0, 180.0, 75.0 };
        else if (v[1])
            p << arr_t<4>{ 0.0, 0.0, 70.0, 0.0 };
        else if (v[2])
            p << arr_t<4>{ 120.0, 90.0, 70.0, 0.0 };
        else if (v[3])
            p << arr_t<4>{ 60.0, 90.0, 180.0, 75.0 };
    }};
}
