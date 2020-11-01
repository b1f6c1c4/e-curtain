#pragma once

#include "common.hpp"
#include <iostream>
#include <fstream>
#include <mutex>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>

template <size_t N>
struct persistent : public sink_source<N> {
    persistent(const std::string &fn) {
        if (mkdir("/var/lib/e-curtain", 0755) < 0)
            std::cout << "Warning: cannot create stateful directory" << std::endl;
        auto f{ "/var/lib/e-curtain/" + fn };
        std::ifstream{ f }.read(reinterpret_cast<char *>(_v.data()), _v.size() * sizeof(double));
        std::ofstream{ f, std::ios::app };
        _f.open(f, std::ios::in | std::ios::out | std::ios::binary);
        if (!_f.is_open() || !_f.good()) {
            std::cout << "Warning: cannot open stateful file: " << std::strerror(errno) << std::endl;
            _f.close();
        }
    }

    sink<N> &operator<<(const arr_t<N> &r) {
        if (_f.is_open()) {
            std::lock_guard l{ _mtx };
            _f.seekp(std::ios::beg);
            if (!_f.good())
                std::cout << "Warning: cannot seekp stateful file: " << std::strerror(errno) << std::endl;
            _f.write(reinterpret_cast<const char *>(r.data()), r.size() * sizeof(double));
            if (!_f.good())
                std::cout << "Warning: cannot write stateful file: " << std::strerror(errno) << std::endl;
            _f.flush();
            if (!_f.good())
                std::cout << "Warning: cannot flush stateful file: " << std::strerror(errno) << std::endl;
        }
        return *this;
    }

    source<N> &operator>>(arr_t<N> &r) {
        r = _v;
        return *this;
    }

private:
    std::mutex _mtx;
    arr_t<N> _v;
    std::fstream _f;
};
