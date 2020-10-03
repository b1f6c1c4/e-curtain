#include "common.hpp"
#include <chrono>
#include <iomanip>
#include "udp_server.hpp"
#include "filter.hpp"
#include "mpc.hpp"
#include "rf.hpp"
#include "sync.hpp"

using namespace std::chrono_literals;

int main() {
    rf rf_inst{"/dev/gpiochip0"};
    synchronizer<0> rf_s{"rf_s", 0s, [&]() {
        arr_t<4> v;
        rf_inst >> v;
        std::cout << v[0] << " ";
        std::cout << v[1] << " ";
        std::cout << v[2] << " ";
        std::cout << v[3] << std::endl;
    }};

    /*

    udp_server<5> udp{ 23333 };
    // Layout:
    // udp_s[0]: t0
    // udp_s[1]: t1
    // udp_s[2]: t2
    // udp_s[3]: silence
    // udp_s[4]: ts1
    // udp_s[5]: ts2
    // udp_s[6]: k
    synchronizer<7> udp_s{"udp_s", 0s};

    t0d_filter t0d;
    synchronizer<1> t0d_s{"t0d_s", 10min, [&](){
        std::cout << now{} << " t0d" << std::endl;
        arr_t<7> v;
        udp_s >> v;
        if (IS_INV(v[0]))
            return;
        t0d << arr_t<1>{ v[0] };
        t0d | t0d_s;
    }};

    mpc m;
    synchronizer<3> mpc_s{"mpc_s", 30s, [&](){
        std::cout << now{} << " mpc" << std::endl;
        arr_t<7> v_udp;
        udp_s >> v_udp;
        arr_t<1> v_t0d;
        t0d_s >> v_t0d;
        m << arr_t<7>{
            v_t0d[0], // t0d
            v_udp[1], // t1
            v_udp[2], // t2
            v_udp[3], // silence
            v_udp[4], // tp1
            v_udp[5], // tp2
            v_udp[6], // k
        };
        m | mpc_s;
    }};

    udp_s.set_callback([&](){
        arr_t<5> v_udp;
        udp >> v_udp;
        std::cout << now{} << " udp" << v_udp[0] << std::endl;
        arr_t<7> v_orig;
        udp_s >> v_orig;
        if (v_udp[0] == 0) {
            v_orig[0] = v_udp[1];
            udp_s << v_orig;
        } else if (v_udp[0] == 1) {
            v_orig[1] = v_udp[1];
            udp_s << v_orig;
        } else if (v_udp[0] == 2) {
            v_orig[2] = v_udp[1];
            udp_s << v_orig;
        } else if (v_udp[0] == 3) {
            v_orig[3] = v_udp[1];
            v_orig[4] = v_udp[2];
            v_orig[5] = v_udp[3];
            v_orig[6] = v_udp[4];
            udp_s << v_orig;
            mpc_s.immediate();
        }
    });

    auto pwm_ticks{ INV };
    auto as_fan{ INV };
    auto as_window{ INV };
    auto as_curtain{ INV };
    auto as_ac{ INV };
    auto as_ac_fan{ INV };
    auto as_acx{ INV };
    synchronizer<0> pwm_s{"pwm_s", 1s, [&](){
        arr_t<3> v_mpc;
        mpc_s >> v_mpc;
    }};
     */
}
