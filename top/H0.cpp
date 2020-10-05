#include "common.hpp"
#include <nlohmann/json.hpp>
#include "net/udp_client.hpp"
#include "sync.hpp"
#include "io/external.hpp"

using nlohmann::json;

int main(int argc, char *argv[]) {
    json j;
    curl{ "https://httpbin.org/ip" } >> j;
    std::cout << j;
}
