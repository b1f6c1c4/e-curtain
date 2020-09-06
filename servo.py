#!/usr/bin/python3

import sys
import RPi.GPIO as GPIO
from time import sleep

def go(port, angle, delay):
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(port, GPIO.OUT)
    GPIO.output(port, GPIO.LOW)
    sleep(0.1)
    f = 100
    p = GPIO.PWM(port, f)
    d = angle/180*(2.38-0.55)+0.55
    if angle < 1:
        d = 0.50
    elif angle > 179:
        d = 2.39
    dc = d*f/10
    p.start(dc)
    sleep(delay)
    p.stop()
    GPIO.cleanup()
    sleep(delay)

port = int(sys.argv[1])
angle = float(sys.argv[2])

go(port, angle, 1.00)
go(port, angle, 0.50)
go(port, angle, 0.25)
