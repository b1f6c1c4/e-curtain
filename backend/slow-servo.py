#!/usr/bin/python3

import sys
import RPi.GPIO as GPIO
from time import sleep

port = 16

def dc(angle):
    return (angle/180*(2.38-0.55)+0.55)*100/10

def goLeft():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(port, GPIO.OUT)
    GPIO.output(port, GPIO.LOW)
    sleep(0.1)
    p = GPIO.PWM(port, 100)
    p.start(dc(0))
    for i in range(0, 76, 1):
        p.ChangeDutyCycle(dc(i))
        sleep(0.03)
    p.stop()
    GPIO.output(port, GPIO.LOW)
    GPIO.cleanup()

def goRight():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(port, GPIO.OUT)
    GPIO.output(port, GPIO.LOW)
    sleep(0.1)
    p = GPIO.PWM(port, 100)
    p.start(dc(75))
    for i in range(75, -1, -1):
        p.ChangeDutyCycle(dc(i))
        sleep(0.03)
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
