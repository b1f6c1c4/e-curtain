#include "common.hpp"
#include <fstream>

constexpr const auto N = 448 / 8;

int main(int argc, char *argv[]) {
    std::string file;
    if (argc == 2) {
        file = argv[1];
    } else {
        std::cout << "Usage: ./audit <file>" << std::endl;
        return 1;
    }

    std::ifstream f(file, std::ios::binary | std::ios::in);

    arr_t<N> buf;
    while (!f.eof()) {
        f.read(reinterpret_cast<char *>(buf.data()), sizeof(double) * N);

        std::chrono::system_clock::duration d{ *reinterpret_cast<uint64_t *>(&buf[0]) };
        std::chrono::system_clock::time_point clk{ d };
        auto itt{ std::chrono::system_clock::to_time_t(clk) };
        std::cout << std::put_time(localtime(&itt), "%FT%T.");
        std::cout << std::setfill('0') << std::setw(9) << (clk.time_since_epoch().count() % 1000000000);
        std::cout << " ";

        auto heat{ buf[1 + 20 + 9 + 2 + 0] };
        auto acm{ buf[1 + 20 + 9 + 2 + 3] };
        auto acp{ buf[1 + 20 + 9 + 2 + 4] };
        auto f012{ buf[1 + 20 + 3] };
        auto t1m0{ buf[1 + 20 + 9 + 0] };
        auto power_in{ 666.0 * heat + 2500.0 * std::max(0.0, acp * acm) - 350.0 * std::min(0.0, t1m0) * f012 };
        auto power_out{ -2500.0 * std::min(0.0, acp * acm) + 350.0 * std::max(0.0, t1m0) * f012 };

        std::cout << std::left << std::setprecision(4) << std::setfill(' ') << std::setw(7);
        std::cout << heat << " ";
        std::cout << std::left << std::setprecision(4) << std::setfill(' ') << std::setw(7);
        std::cout << acm << " ";
        std::cout << std::left << std::setprecision(4) << std::setfill(' ') << std::setw(7);
        std::cout << f012 << " ";
        std::cout << std::left << std::setprecision(4) << std::setfill(' ') << std::setw(7);
        std::cout << t1m0 << " ";
        std::cout << std::left << std::setprecision(4) << std::setfill(' ') << std::setw(7);
        std::cout << power_in << " ";
        std::cout << std::left << std::setprecision(4) << std::setfill(' ') << std::setw(7);
        std::cout << power_out << " ";
        std::cout << std::left << std::setprecision(4) << std::setfill(' ') << std::setw(7);
        std::cout << std::endl;
    }
}
