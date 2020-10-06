#include "common.hpp"
#include <mutex>
#include <fstream>
#include <nlohmann/json.hpp>
#include "net/udp_server.hpp"
#include "net/udp_client.hpp"
#include "io/external.hpp"
#include "libdumbac.h"
#include "sync.hpp"

using namespace std::chrono_literals;

int main(int argc, char *argv[]) {
    std::string host1{ "controller-1" };
    std::string host2{ "controller-2" };
    if (argc == 1) {
        // use default hosts
    } else if (argc == 3) {
        host1 = argv[1];
        host2 = argv[2];
    } else {
        std::cout << "Usage: ./C [<host1> <host2>]" << std::endl;
        std::cout << "Note: The default <host1> is controller-1" << std::endl;
        std::cout << "Note: The default <host2> is controller-2" << std::endl;
        return 1;
    }


    nlohmann::json j;
    std::ifstream{ "/etc/e-curtain/weather.json" } >> j;
    auto api_key{ j["key"].get<std::string>() };
    auto lat{ j["lat"].get<double>() };
    auto lon{ j["lon"].get<double>() };
    auto url{ std::string("https://api.openweathermap.org/data/2.5/onecall") +
              "?lat=" + std::to_string(lat) +
              "&lon=" + std::to_string(lon) +
              "&exclude=" + "daily,hourly,minutely,alerts" +
              "&appid=" + api_key };

    udp_client<7> i_udp_client{ "localhost", PORT };
    synchronizer<0> s_t0{ "s_t0", 10min, [&]() {
        curl{ url } >> j;

        arr_t<7> v;
        v[0] = 0; // tag
        v[1] = j["current"]["temp"].get<double>() - 273.15;
        v[2] = j["current"]["humidity"].get<double>();

        auto dt{ j["current"]["dt"].get<int>() };
        auto ton{ j["current"]["sunrise"].get<int>() };
        auto toff{ j["current"]["sunset"].get<int>() };
        v[3] = (dt - ton) / 3600.0; // hours since sunrise
        v[4] = (toff - dt) / 3600.0; // hours before sunset
        v[5] = j["current"]["uvi"].get<double>();
        v[6] = j["current"]["wind_speed"].get<double>() <= 5.5; // f012bu

        std::cout << "Info: Got weather: " << v << std::endl;
        i_udp_client << v;
    } };


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
                std::cout << "Warning: invalid udp package type" << std::endl;
        }
    } };

    udp_client<3> i_udp_client1{ host1, PORT };
    udp_client<4> i_udp_client2{ host2, PORT };

    std::ofstream logger{ "/var/log/e-curtain.bin", std::ios_base::app };

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
    } };
}
