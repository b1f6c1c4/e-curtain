#!/usr/bin/python3

import sys
import RPi.GPIO as GPIO
from time import sleep

port_ena = 12
port_pul = 11
port_dir = 13

def goLeft():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(port_pul, GPIO.OUT)
    GPIO.setup(port_dir, GPIO.OUT)
    GPIO.setup(port_ena, GPIO.OUT)
    GPIO.output(port_dir, GPIO.LOW)
    GPIO.output(port_ena, GPIO.HIGH)
    p = GPIO.PWM(port_pul, 1000)
    p.start(50)
    sleep(5.0)
    p.stop()
    GPIO.cleanup()

def goRight():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(port_pul, GPIO.OUT)
    GPIO.setup(port_dir, GPIO.OUT)
    GPIO.setup(port_ena, GPIO.OUT)
    GPIO.output(port_dir, GPIO.HIGH)
    GPIO.output(port_ena, GPIO.HIGH)
    p = GPIO.PWM(port, 1000)
    p.start(50)
    sleep(5.0)
    p.stop()
    GPIO.cleanup()

if __name__ == "__main__":
    if sys.argv[1] == "left":
        goLeft()
    elif sys.argv[1] == "right":
        goRight()
    else:
        print("what are you trying to do?")
