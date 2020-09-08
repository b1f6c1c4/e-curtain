#!/usr/bin/python3

import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BOARD)
GPIO.setup(40, GPIO.IN)
GPIO.setup(38, GPIO.IN)
GPIO.setup(36, GPIO.IN)
GPIO.setup(32, GPIO.IN)
while True:
    d = GPIO.input(40)
    c = GPIO.input(38)
    b = GPIO.input(36)
    a = GPIO.input(32)
    v = a << 3 | b << 2 | c << 1 | d
    if v != 0:
        print(v)
        break
    sleep(0.1)

GPIO.cleanup()
