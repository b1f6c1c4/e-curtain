#include "common.hpp"
#include "net/udp_client.hpp"
#include "dsp/filter.hpp"
#include "io/rf.hpp"


int main() {
    // rf rf_inst{ "/dev/gpiochip0" };

    udp_client<6> udp{ "controller-2", PORT };
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        arr_t<4> v;
        // rf_inst >> v;
        std::cout << "Key pressed: ";
        std::cout << v[0] << " ";
        std::cout << v[1] << " ";
        std::cout << v[2] << " ";
        std::cout << v[3] << std::endl;
        sleep(1000);
        // TODO
    };
#pragma clang diagnostic pop
}
