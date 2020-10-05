#include "common.hpp"
#include "net/udp_server.hpp"
#include "io/servo.hpp"
#include "sync.hpp"

using namespace std::chrono_literals;

int main(int argc, char *argv[]) {
    std::array<servo, 4> p{ 10, 12, 8, 16 };
    p << arr_t<4>{ 120.0, 180.0, 0.0, 0.0 };
    sleep(100);
}
