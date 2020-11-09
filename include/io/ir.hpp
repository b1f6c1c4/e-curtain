#pragma once

#include "common.hpp"

struct ir : public sink<1> {
    ir(const std::string &dev, int port_tx, int port_chk);

    sink<1> &operator<<(const arr_t<1> &r);

private:
    static constexpr size_t max_cycles{ 240 };
    int _chipfd, _ifd, _ofd;
    int _state;
    size_t _cycle;

    [[nodiscard]] bool check() const;
    void issue(const arr_t<12> &r);
    [[nodiscard]] bool power();
};
