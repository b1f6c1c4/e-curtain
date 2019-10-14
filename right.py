#!/usr/bin/python3

import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BOARD)
GPIO.setup(7, GPIO.OUT)

p = GPIO.PWM(7, 1 / 3000e-6)
p.start(25)
sleep(10.7)
p.stop()

GPIO.output(7, GPIO.LOW)

GPIO.cleanup()
