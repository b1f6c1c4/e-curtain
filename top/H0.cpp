#include "common.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include "net/udp_client.hpp"
#include "sync.hpp"
#include "io/external.hpp"

using namespace std::chrono_literals;
namespace fs = std::filesystem;

using nlohmann::json;

int main(int argc, char *argv[]) {
    std::string host{ "controller-2" };
    if (argc == 1) {
        // use default host
    } else if (argc == 2) {
        host = argv[1];
    } else {
        std::cout << "Usage: ./H0 [<host>]" << std::endl;
        std::cout << "Note: The default <host> is controller-2" << std::endl;
        return 1;
    }

    json j;
    std::ifstream{ fs::weakly_canonical(fs::path(argv[0])).parent_path().parent_path() / "weather.json" } >> j;
    auto api_key{ j["key"].get<std::string>() };
    auto lat{ j["lat"].get<double>() };
    auto lon{ j["lon"].get<double>() };
    auto url{ std::string("https://api.openweathermap.org/data/2.5/onecall") +
              "?lat=" + std::to_string(lat) +
              "&lon=" + std::to_string(lon) +
              "&exclude=" + "daily,hourly,minutely,alerts" +
              "&appid=" + api_key };

    udp_client<6> i_udp_client{ host, PORT };
    synchronizer<0> s_t0{ "s_t0", 10min, [&]() {
        curl{ url } >> j;

        arr_t<6> v;
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

        std::cout << "weather -> " << host << ": " << v << std::endl;
        i_udp_client << v;
    }};
}
