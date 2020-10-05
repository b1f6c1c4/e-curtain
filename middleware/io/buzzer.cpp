#include "io/buzzer.hpp"

buzzer::buzzer() {
    *this << R"py(
import code
code.interact(local=locals())

import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BOARD)
GPIO.setup(11, GPIO.OUT)

p = GPIO.PWM(11, 1500)
)py" << std::flush;
}

void buzzer::on() {
    _l = std::make_unique<lock_shared<pwm_chan>>(g_pwm);
    *this << "p.start(50)\n" << std::flush;
}

void buzzer::off() {
    _l = nullptr;
    *this << "p.stop()\n" << std::flush;
}
