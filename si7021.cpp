#include "si7021.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdexcept>

si7021::si7021(const std::string &dev) : _fd{ open(dev.c_str(), O_RDWR) } {
    if (_fd < 0)
        throw std::runtime_error("Cannot open dev");
}

si7021::~si7021() {
    if (_fd >= 0)
        close(_fd);
}

source<2> &si7021::operator>>(arr_t<2> &r) {
    unsigned char addr[1], buf[2];

    i2c_msg ioctl_msg[2];
    ioctl_msg[0].len = 1;
    ioctl_msg[0].addr = 0x40;
    ioctl_msg[0].buf = addr;
    ioctl_msg[0].flags = 0;
    ioctl_msg[1].len = 1;
    ioctl_msg[1].addr = 0x40;
    ioctl_msg[1].buf = buf;
    ioctl_msg[1].flags = I2C_M_RD;

    i2c_rdwr_ioctl_data ioctl_data{};
    ioctl_data.nmsgs = 2;
    ioctl_data.msgs = ioctl_msg;

    addr[0] = 0xe5;
    if (ioctl(_fd, I2C_RDWR, &ioctl_data) < 0)
        throw std::runtime_error("Cannot read from i2c bus");

    auto v = static_cast<unsigned int>(buf[0]) << 8u | static_cast<unsigned int>(buf[1]);
    r[1] = 125 * static_cast<double>(v) / 65536 - 6;
    v = static_cast<unsigned int>(buf[0]) << 8u | static_cast<unsigned int>(buf[1]);

    addr[0] = 0xe0;
    if (ioctl(_fd, I2C_RDWR, &ioctl_data) < 0)
        throw std::runtime_error("Cannot read from i2c bus");
    r[0] = 175.72 * v / 65536 - 46.85;
    return *this;
}
