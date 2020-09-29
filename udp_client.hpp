#pragma once

#include "common.h"
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>

template <size_t N>
struct udp_client : public sink<N> {
    udp_client(const std::string &target, int port)
            : _fd{ socket(AF_INET, SOCK_DGRAM, 0) }, _target{} {
        _target.sin_family = AF_INET;
        _target.sin_port = htons(port);
        inet_pton(AF_INET, target.c_str(), &_target.sin_addr);
    }

    sink<N> operator<<(const arr_t<N> &r) override {
        sendto(_fd, reinterpret_cast<const char *>(r.data()), sizeof(double) * r.size(),
               reinterpret_cast<const sockaddr *>(&_target), sizeof(_target));
    }

private:
    int _fd;
    sockaddr_in _target;
};
