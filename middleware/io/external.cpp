#include "io/external.hpp"
#include <ostream>

external_fp::external_fp(const std::string &cmd, bool rw) : stdio_filebuf<char>{
        popen(cmd.c_str(), rw ? "r" : "w"),
        rw ? std::ios_base::in : std::ios_base::out,
} {
    if (!file())
        throw std::runtime_error("Cannot open external");
}

external_fp::~external_fp() {
    if (file()) {
        pclose(file());
    }
}

external_in::external_in(const std::string &cmd) : external_fp{ cmd, true }, std::istream{ this } { }

external_out::external_out(const std::string &cmd) : external_fp{ cmd, false }, std::ostream{ this } { }

curl::curl(const std::string &url) : external_in{ "curl -fsSL '" + url + "'" } { }

py::py() : external_out{ "python3 -c 'import code\ncode.interact(local=locals())' >/dev/null 2>/dev/null" } { }
