#!/usr/bin/python3

import RPi.GPIO as GPIO
from time import sleep

def goLeft():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(3, GPIO.OUT)
    p = GPIO.PWM(3, 1 / 3000e-6)
    p.start(75)
    sleep(8.0)
    p.stop()
    GPIO.output(3, GPIO.LOW)
    GPIO.cleanup()

def goRight():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(3, GPIO.OUT)
    p = GPIO.PWM(3, 1 / 3000e-6)
    p.start(25)
    sleep(7.9)
    p.stop()
    GPIO.output(3, GPIO.LOW)
    GPIO.cleanup()

if __name__ == "__main__":
    if sys.argv[0] == "left":
        goLeft()
    elif sys.argv[0] == "right":
        goRight()
