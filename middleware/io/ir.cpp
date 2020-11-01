#include "io/ir.hpp"
#include "io/external.hpp"

ir::ir(const std::string &dev, int port)

sink<12> &ir::operator<<(const arr_t<12> &r) {
    external_out e{ "ir-ctl -d " + dev + " --send /dev/stdin" };
    e << "carrier 37500" << std::endl;
    for (size_t i{ 0 }; i < r.size(); i++) {
        auto len = r[i] ? 1260 : 420;
        e << "pulse " << len << std::endl;
        if (i != r.size() - 1)
            e << "space " << 1700 - len << std::endl;
    }
    e.flush();
}