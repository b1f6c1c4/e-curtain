#include "si7021.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdexcept>

si7021::si7021(const std::string &dev) : _fd{ open(dev.c_str(), O_RDWR) } {
    if (_fd < 0)
        throw std::runtime_error("Cannot open dev");

    if (ioctl(_fd, I2C_SLAVE, 0x40) < 0)
        throw std::runtime_error("Cannot acquire bus access");
}

si7021::~si7021() {
    if (_fd >= 0)
        close(_fd);
}

source<2> &si7021::operator>>(arr_t<2> &r) {
    char buf[2];
    if (read(_fd, buf, 2) != 2)
        throw std::runtime_error("Cannot read from i2c bus");
    auto v = static_cast<unsigned int>(buf[0]) << 8u | static_cast<unsigned int>(buf[1]);
    r[1] = 125 * static_cast<double>(v) / 65536 - 6;
    v = static_cast<unsigned int>(buf[0]) << 8u | static_cast<unsigned int>(buf[1]);
    if (read(_fd, buf, 2) != 2)
        throw std::runtime_error("Cannot read from i2c bus");
    r[0] = 175.72 * v / 65536 - 46.85;
    return *this;
}
