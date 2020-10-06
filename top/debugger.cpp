#include "common.hpp"
#include <iostream>
#include "net/udp_client.hpp"

int main(int argc, char *argv[]) {
    std::string host;
    if (argc == 2) {
        host = argv[1];
    } else {
        std::cout << "Usage: ./debugger <host>" << std::endl;
        return 1;
    }

    constexpr const size_t max{ 576 / sizeof(double) };
    udp_client<max> i_udp_client{ host, PORT };

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss{ line };
        arr_t<max> v;
        double n;
        size_t cnt{ 0 };
        for (; iss >> n; cnt++)
            v[cnt] = n;
        std::cout << "Sending [";
        for (size_t i{ 0 }; i < cnt; i++) {
            std::cout << v[i];
            if (i < cnt - 1)
                std::cout << ", ";
        }
        std::cout << "] to " << host <<  ":" << PORT << std::endl;
        i_udp_client << v;
    }
}
