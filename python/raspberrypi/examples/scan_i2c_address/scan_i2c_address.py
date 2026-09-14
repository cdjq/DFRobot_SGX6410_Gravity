# -*- coding:utf-8 -*-
'''!
  @file scan_i2c_address.py
  @brief Scan the I2C bus for Gravity SGX6410
  @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license The MIT License (MIT)
  @author PLELES(li.jia@dfrobot.com)
  @version V1.0.0
  @date 2026-09-12
  @url https://github.com/DFRobot/DFRobot_SGX6410_Gravity
'''
import os
import sys
import time
try:
  import smbus
except:
  import smbus2 as smbus

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))))
from DFRobot_SGX6410_Gravity import *

I2C_1 = 0x01

def setup():
  print("======== Gravity SGX6410 scanI2CAddress ========")
  print("Expected  : 0x52 (ADD_SEL low) or 0x53 (ADD_SEL high)")
  print("==================================================")

def loop():
  found = 0
  print("Scanning...")
  bus = smbus.SMBus(I2C_1)
  addr = 1
  while addr < 127:
    try:
      bus.write_quick(addr)
      found = found + 1
      text = "Found 0x%02X"%(addr)
      if addr == 0x52:
        text = text + "  (SGX6410 ADD_SEL low)"
      elif addr == 0x53:
        text = text + "  (SGX6410 ADD_SEL high)"
      print(text)
    except:
      pass
    addr = addr + 1
  if found == 0:
    print("No I2C device found, check wiring and COM_SEL")
  else:
    print("Devices found: %d"%(found))
  print("--------------------")
  time.sleep(2)

if __name__ == "__main__":
  setup()
  while True:
    loop()
