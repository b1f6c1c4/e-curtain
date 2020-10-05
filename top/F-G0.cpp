#include "common.hpp"
#include "net/udp_client.hpp"
#include "net/udp_server.hpp"
#include "dsp/filter.hpp"
#include "io/gpio.hpp"
#include "sync.hpp"

using namespace std::chrono_literals;

int main() {
    gpio<4, 4> rf_servo{ "/dev/gpiochip0", { 12, 16, 20, 21 }, { 15, 18, 14, 23 }, { false, false, false, true } };

    udp_server<3> udp_s{ PORT };
    udp_client<6> udp_c{ "controller-2", PORT };

    synchronizer<4> rf_s{ "rf_s", 0s, [&]() {
        arr_t<4> v;
        rf_servo >> v;
        std::cout << "Key pressed: ";
        std::cout << v[0] << " ";
        std::cout << v[1] << " ";
        std::cout << v[2] << " ";
        std::cout << v[3] << std::endl;
        if (v[0])
            rf_servo << arr_t<4>{ 180.0, 180.0, 180.0, 75.0 };
        else if (v[1])
            rf_servo << arr_t<4>{ 0.0, 0.0, 70.0, 0.0 };
        else if (v[2])
            rf_servo << arr_t<4>{ 120.0, 90.0, 70.0, 0.0 };
        else if (v[3])
            rf_servo << arr_t<4>{ 60.0, 90.0, 180.0, 75.0 };
    }};
}
