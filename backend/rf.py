#!/usr/bin/python3

import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BOARD)
GPIO.setup(40, GPIO.IN)
GPIO.setup(38, GPIO.IN)
GPIO.setup(36, GPIO.IN)
GPIO.setup(32, GPIO.IN)
d = GPIO.input(40)
c = GPIO.input(38)
b = GPIO.input(36)
a = GPIO.input(32)
print(a << 3 | b << 2 | c << 1 | d)
GPIO.cleanup()
