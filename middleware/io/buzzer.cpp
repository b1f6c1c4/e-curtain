#include "io/buzzer.hpp"

buzzer::buzzer() : _fp{ popen("python3 -c 'import code\ncode.interact(local=locals())'", "w") } {
    if (!_fp)
        throw std::runtime_error("Cannot open python");
    write(R"py(
import code
code.interact(local=locals())

import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BOARD)
GPIO.setup(11, GPIO.OUT)

p = GPIO.PWM(11, 1500)
)py");
}

buzzer::~buzzer() {
    if (_fp) {
        pclose(_fp);
        _fp = nullptr;
    }
}

void buzzer::on() {
    write("p.start(50)\n");
}

void buzzer::off() {
    write("p.stop()\n");
}

void buzzer::write(const std::string &str) {
    std::cout << "Writing python: " << str; // must end with \n
    if (fwrite(reinterpret_cast<const void *>(str.c_str()), sizeof(char), str.size(), _fp) != str.size())
        throw std::runtime_error("Cannot use python");
    if (fflush(_fp) < 0)
        throw std::runtime_error("Cannot use python");
}
