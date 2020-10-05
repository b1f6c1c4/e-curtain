#include "common.hpp"
#include <mutex>
#include <filesystem>
#include <fstream>
#include "net/udp_server.hpp"
#include "net/udp_client.hpp"
#include "libdumbac.h"
#include "sync.hpp"

using namespace std::chrono_literals;
namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    std::string host1{ "controller-1" };
    std::string host2{ "controller-2" };
    if (argc == 1) {
        // use default hosts
    } else if (argc == 3) {
        host1 = argv[1];
        host2 = argv[2];
    } else {
        std::cerr << "Usage: ./C [<host1> <host2>]" << std::endl;
        std::cerr << "Note: The default <host1> is controller-1" << std::endl;
        std::cerr << "Note: The default <host2> is controller-2" << std::endl;
        return 1;
    }

    std::mutex mtx{};
    udp_server<10> i_udp_server{ PORT };
    libdumbacModelClass::ExtU u;
    arr_t<3> h;
    arr_t<2> f012bu;
    synchronizer<0> s_udp{ "s_udp", 0s, [&]() {
        arr_t<10> v;
        i_udp_server >> v;
        std::lock_guard l{ mtx };
        switch (static_cast<int>(v[0])) {
            case 0:
                u.t0 = v[1];
                h[0] = v[2];
                u.sun[0] = v[3]; // ton
                u.sun[1] = v[4]; // toff
                u.sun[2] = v[5]; // uvi
                f012bu[0] = v[6];
                break;
            case 1:
                u.y[0] = v[1];
                h[1] = v[2];
                break;
            case 2:
                u.y[1] = v[1];
                h[2] = v[2];
                break;
            case 3:
                u.tp[0] = v[1];
                u.tp[1] = v[2];
                u.f012b[0] = v[3];
                f012bu[1] = v[4];
                u.curb[0] = v[5];
                u.curb[1] = v[6];
                u.w0 = v[7];
                u.w1 = v[8];
                u.w2 = v[9];
                break;
            default:
                std::cerr << "Warning: invalid udp package type" << std::endl;
        }
    }};

    udp_client<3> i_udp_client1{ host1, PORT };
    udp_client<4> i_udp_client2{ host2, PORT };

    std::ofstream logger{
            fs::weakly_canonical(fs::path(argv[0])).parent_path().parent_path() / "e-curtain-cxx.log.bin",
            std::ios_base::app,
    };

    libdumbacModelClass i_mpc;
    i_mpc.initialize();
    synchronizer<0> s_mpc{ "s_mpc", 10s, [&]() {
        auto ar{ [&]() {
            std::lock_guard l{ mtx };
            u.f012b[1] = std::max(std::min(f012bu[0], f012bu[1]), u.f012b[0]);
            i_mpc.setExternalInputs(&u);
            return std::array{
                    u.t0,
                    u.y[0],
                    u.y[1],
                    u.tp[0],
                    u.tp[1],
                    h[0],
                    h[1],
                    h[2],
                    u.sun[0],
                    u.sun[1],
                    u.sun[2],
                    f012bu[0],
                    f012bu[1],
                    u.f012b[0],
                    u.f012b[1],
                    u.curb[0],
                    u.curb[1],
                    u.w0,
                    u.w1,
                    u.w2,
            };
        }() };
        logger.write(reinterpret_cast<const char *>(ar.data()), ar.size());
        logger.flush();

        i_mpc.step();

        auto &res{ i_mpc.getExternalOutputs() };
        logger.write(reinterpret_cast<const char *>(&res), sizeof(res));
        logger.flush();

        arr_t<3> g1;
        arr_t<4> g2;
        g1[2] = res.g[0]; // reg1
        g1[3] = res.g[1]; // reg2
        g1[0] = res.g[2]; // acm
        g1[1] = res.g[3]; // acp
        g2[2] = res.g[4]; // fan
        g2[1] = res.g[5]; // win
        g2[0] = res.g[6]; // cur

        i_udp_client1 << g1;
        i_udp_client2 << g2;
    }};
}
