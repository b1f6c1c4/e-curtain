#!/usr/bin/python3

from http.server import HTTPServer, BaseHTTPRequestHandler
from os import curdir, sep
import datetime
import time
import os
import threading
import RPi.GPIO as GPIO
from time import sleep

def goLeft(t):
    sleep(t * 60)
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(7, GPIO.OUT)
    p = GPIO.PWM(7, 1 / 3000e-6)
    p.start(75)
    sleep(10.0)
    p.stop()
    GPIO.output(7, GPIO.LOW)
    GPIO.cleanup()

def goRight(t):
    sleep(t * 60)
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(7, GPIO.OUT)
    p = GPIO.PWM(7, 1 / 3000e-6)
    p.start(25)
    sleep(10.7)
    p.stop()
    GPIO.output(7, GPIO.LOW)
    GPIO.cleanup()

def die():
    time.sleep(1)
    os._exit(114)

class S(BaseHTTPRequestHandler):
    def _set_headers(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()

    def do_GET(self):
        if (self.path == "/"):
            self.path = "/index.html"
        try:
            f = open(curdir + sep + self.path, 'rb')
            self._set_headers()
            self.wfile.write(f.read())
            f.close()
            return
        except IOError:
            self.send_error(404, 'File Not Found: %s' % self.path)

    def do_HEAD(self):
        self._set_headers()

    def postData(self):
        t = self.rfile.read(int(self.headers['Content-Length'])).decode("utf-8")
        if t == "now":
            return 0.0
        elif t.startswith("+"):
            return float(t)
        else:
            tx = datetime.datetime.strptime(t, '%H:%M')
            tn = datetime.datetime.now()
            if tx.hour > tn.hour or tx.hour == tn.hour and tx.minute > tn.minute:
                tn = datetime.datetime.combine(datetime.date(1900, 1, 1), datetime.time(tn.hour, tn.minute))
                t = tx - tn;
            else:
                tn = datetime.datetime.combine(datetime.date(1900, 1, 1), datetime.time(tn.hour, tn.minute))
                t = tx - tn + datetime.timedelta(days=1);
            return t.total_seconds() / 60

    def do_POST(self):
        if self.path == "/open":
            tv = self.postData()
            content = "opening %d mins later" % tv
            th = threading.Thread(target=goLeft, args=(tv,))
            th.start()
        elif self.path == "/close":
            tv = self.postData()
            content = "closing %d mins later" % tv
            th = threading.Thread(target=goRight, args=(tv,))
            th.start()
        elif self.path == "/cancel":
            content = "Cancelling"
            th = threading.Thread(target=die)
            th.start()
        else:
            self.send_error(404, 'File Not Found: %s' % self.path)
            return
        self._set_headers()
        self.wfile.write(content.encode("utf-8"))


def run(server_class=HTTPServer, handler_class=S, addr="localhost", port=8000):
    server_address = (addr, port)
    httpd = server_class(server_address, handler_class)

    print(f"Starting httpd server on {addr}:{port}")
    httpd.serve_forever()


if __name__ == "__main__":
    run(addr="0.0.0.0", port=8000)
