#!/usr/bin/python3

import sys
import RPi.GPIO as GPIO
from time import sleep

port = 7

def goLeft():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(port, GPIO.OUT)
    p = GPIO.PWM(port, 1 / 3000e-6)
    p.start(75)
    sleep(8.0)
    p.stop()
    GPIO.output(port, GPIO.LOW)
    GPIO.cleanup()

def goRight():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(port, GPIO.OUT)
    p = GPIO.PWM(port, 1 / 3000e-6)
    p.start(25)
    sleep(7.9)
    p.stop()
    GPIO.output(port, GPIO.LOW)
    GPIO.cleanup()

if __name__ == "__main__":
    if sys.argv[1] == "left":
        goLeft()
    elif sys.argv[1] == "right":
        goRight()
    else:
        print("what are you trying to do?")
