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
    std::string host0;
    std::string host1;
    if (argc == 3) {
        host0 = argv[1];
        host1 = argv[2];
    } else {
        std::cout << "Usage: ./C <host0> <host1>" << std::endl;
        return 1;
    }


    std::ofstream logger{ "/var/log/e-curtain.bin", std::ios_base::app };
    constexpr const size_t log_entry{
            sizeof(std::chrono::system_clock::rep) +
            (5 + 3 + 3 + 2 + 2 + 2 + 3) * sizeof(double) +
            sizeof(libdumbacModelClass::ExtY)
    };
    constexpr const size_t log_align{ 64 }; // bytes
    constexpr const size_t log_padding{ (log_entry + 8 + log_align - 1) / log_align * log_align - log_entry };
    constexpr const std::array<char, log_padding> log_pad{ []() constexpr {
        std::array<char, log_padding> res{};
        for (auto &v : res)
            v = 0x5a;
        return res;
    }() };
    std::cout << "Info: length of log entry is " << log_entry;
    std::cout << " / " << log_entry + log_padding << " bytes " << std::endl;


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

    udp_client<4> i_udp_client0{ host0, PORT };
    udp_client<3> i_udp_client1{ host1, PORT };

    libdumbacModelClass i_mpc;
    i_mpc.initialize();
    synchronizer<0> s_mpc{ "s_mpc", 10s, [&]() {
        auto clk{ std::chrono::system_clock::now().time_since_epoch().count() };
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

        i_mpc.step();

        auto &res{ i_mpc.getExternalOutputs() };
        logger.write(reinterpret_cast<const char *>(&clk), sizeof(clk));
        logger.write(reinterpret_cast<const char *>(ar.data()), ar.size() * sizeof(double));
        logger.write(reinterpret_cast<const char *>(&res), sizeof(res));
        logger.write(log_pad.data(), log_padding);
        logger.flush();

        arr_t<4> g0;
        arr_t<3> g1;
        g0[2] = res.g[0]; // reg1
        g0[3] = res.g[1]; // reg2
        g0[0] = res.g[2]; // acm
        g0[1] = res.g[3]; // acp
        g1[2] = res.g[4]; // fan
        g1[1] = res.g[5]; // win
        g1[0] = res.g[6]; // cur

        i_udp_client0 << g0;
        i_udp_client1 << g1;
    } };
}