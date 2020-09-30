#pragma once

#include "common.hpp"
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>

template <size_t N>
struct udp_server : public source<N> {
    udp_server(int port)
            : _fd{ socket(AF_INET, SOCK_DGRAM, 0) }, _target{} {
        if (_fd < 0)
            throw std::runtime_error("Cannot open socket");
        _target.sin_family = AF_INET;
        _target.sin_addr.s_addr = INADDR_ANY;
        _target.sin_port = htons(port);
        if (bind(_fd, reinterpret_cast<const sockaddr *>(&_target), sizeof(_target)) < 0)
            throw std::runtime_error("Cannot bind socket");
    }

    ~udp_server() {
        if (_fd >= 0)
            close(_fd);
    };

    source<N> &operator>>(arr_t<N> &r) override {
        recvfrom(_fd, reinterpret_cast<char *>(r.data()), sizeof(double) * r.size(), MSG_WAITALL, nullptr, 0);
        return *this;
    }

private:
    int _fd;
    sockaddr_in _target;
};
