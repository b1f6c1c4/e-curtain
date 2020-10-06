#include "common.hpp"
#include <mutex>
#include "net/udp_client.hpp"
#include "net/udp_server.hpp"
#include "dsp/pwm.hpp"
#include "dsp/overlay.hpp"
#include "io/gpio.hpp"
#include "sync.hpp"

using namespace std::chrono_literals;

int main(int argc, char *argv[]) {
    std::string host;
    if (argc == 2) {
        host = argv[1];
    } else {
        std::cout << "Usage: ./F-G0 <host>" << std::endl;
        return 1;
    }

    std::mutex mtx;
    gpio<4, 4> i_gpio{
            "/dev/gpiochip0",
            // Inputs:
            // A   B   C   D
            // 32  36  38  40
            { 12, 16, 20, 21 },
            // Outputs:
            // acp acm r1  r2
            // 12  10  8   16
            { 18, 15, 14, 23 },
            // Output Slew Limits:
            { false, false, false, true },
    };
    overlay<4> i_overlay{};

    auto write{ [&](double acp, double acm, double reg1, double reg2) {
        std::cout << "Write: " << acp << " " << acm << " " << reg1 << " " << reg2 << std::endl;
        auto m{ 60.0 };
        auto p{ 180.0 * acp };
        switch (static_cast<int>(acm)) {
            case 2:
                p = 180.0 - p;
                m = 0.0;
                break;
            case 1:
                p = 180.0 - p;
                m = 30.0;
                break;
            case 0:
                m = 60.0;
                break;
            case -1:
                m = 150.0;
                break;
            case -2:
                m = 180.0;
                break;
            default:
                std::cout << "Warning: Invalid acm" << std::endl;
                break;
        }
        i_overlay << arr_t<4>{ p, m, 180.0 - 100.0 * reg1, 77.0 * reg2 };
        i_overlay | i_gpio;
    } };

    udp_client<6> i_udp_client{ host, PORT };
    synchronizer<4> s_sp{ "s_sp", 0s, [&]() {
        arr_t<4> v;
        i_gpio >> v;
        // TODO: set point
        // i_udp_client << sp;
    } };

    udp_server<4> i_udp_server{ PORT };
    pwm i_pwm{ 60 }; // 60 x 2s
    synchronizer<1> s_pwm{ "s_pwm", 2s, };
    synchronizer<3> s_recv{ "s_recv", 0s, [&]() {
        arr_t<4> v;
        i_udp_server >> v;
        i_pwm << arr_t<1>{ v[1] }; // acp
        s_recv << arr_t<3>{ v[0], v[2], v[3] }; // acm reg1 reg2
        arr_t<1> pv;
        s_pwm >> pv;
        std::lock_guard l{ mtx };
        write(pv[0], v[0], v[2], v[3]);
    } };
    s_recv << arr_t<3>{ 0.0, 0.0, 0.0 };
    s_pwm.set_callback([&]() {
        arr_t<1> v;
        i_pwm >> v;
        if (!IS_INV(v[0])) {
            s_pwm << v;
            arr_t<3> sv;
            s_recv >> sv;
            std::lock_guard l{ mtx };
            write(v[0], sv[0], sv[1], sv[2]);
        }
    });
}
