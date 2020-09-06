#!/usr/bin/python3

import smbus

bus = smbus.SMBus(1)
address = 0x40

rh = bus.read_word_data(address, 0xe5)
rh = (rh & 0xff) << 8 | rh >> 8
rh = 125 * rh / 65536 - 6
print(rh)

t = bus.read_word_data(address, 0xe0)
t = (t & 0xff) << 8 | t >> 8
t = 175.72 * t / 65536 - 46.85
print(t)
