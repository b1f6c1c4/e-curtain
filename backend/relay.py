#!/usr/bin/python3

import sys
import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BOARD)

if int(sys.argv[1]):
    GPIO.setup(10, GPIO.OUT)
    GPIO.output(7, 1)
    sleep(0.3)
    GPIO.output(7, 0)
else:
    GPIO.setup(8, GPIO.OUT)
    GPIO.output(10, 1)
    sleep(0.3)
    GPIO.output(10, 0)

GPIO.cleanup()
