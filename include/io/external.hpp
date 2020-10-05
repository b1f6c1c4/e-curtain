#pragma once

#include "common.hpp"
#include <ext/stdio_filebuf.h>
#include <istream>
#include <ostream>

using __gnu_cxx::stdio_filebuf;

struct external_fp : public stdio_filebuf<char> {
    external_fp(const std::string &cmd, bool rw);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-override"

    ~external_fp();

#pragma clang diagnostic pop
};

struct external_in : protected external_fp, public std::istream {
    explicit external_in(const std::string &cmd);
};

struct external_out : protected external_fp, public std::ostream {
    explicit external_out(const std::string &cmd);
};

struct curl : public external_in {
    curl(const std::string &url);
};

struct py : public external_out {
    py();
};
