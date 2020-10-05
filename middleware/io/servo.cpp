#include "io/servo.hpp"
#include <thread>
#include "io/shared.hpp"

using namespace std::chrono_literals;

servo::servo(int pin) : _pin{ pin } { }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-unused-raii"
sink<1> &servo::operator<<(const arr_t<1> &r) {
    auto d{ r[0]/180*(2.38-0.55)+0.55 }; // duration in ms
    if (r[0] < 1)
        d = 0.50;
    else if (r[0] > 179)
        d = 2.39;
    auto dc{ d*freq/10 };

    _th = std::thread{[this, dc](){
        lock_shared l{ g_pwm };
        {
            py{} << R"py(
import RPi.GPIO as GPIO
from time import sleep
port = )py" << _pin << R"py(
GPIO.setmode(GPIO.BOARD)
GPIO.setup(port, GPIO.OUT)
p = GPIO.PWM(port, )py" << freq << R"py()
p.start()py" << dc << R"py()
sleep(0.4)
p.stop()
sleep(0.1)
p.start()py" << dc << R"py()
sleep(0.4)
p.stop()
GPIO.cleanup()
)py";
        }
        std::this_thread::sleep_for(1.5s);
    }};
    return *this;
}
#pragma clang diagnostic pop
