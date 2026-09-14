# -*- coding:utf-8 -*-
'''!
  @file run_sensor_clean.py
  @brief Run the Gravity SGX6410 thermal clean once
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

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))))
from DFRobot_SGX6410_Gravity import *

ctype = 0
ADDRESS = SGX_ADDR_DEFAULT
I2C_1   = 0x01

if ctype == 0:
  sgx6410 = DFRobot_SGX6410_Gravity_I2C(I2C_1, ADDRESS)
else:
  sgx6410 = DFRobot_SGX6410_Gravity_UART(SGX_UART_BAUD_DEFAULT, ADDRESS)

def setup():
  # Initialize the module and verify VID 0x3343
  while (sgx6410.begin() == False):
    print("Init failed, check address, COM_SEL and wiring")
    time.sleep(1)

  # Clean once in the sensor lifetime. Keep power stable for about 60 s
  result = sgx6410.run_sensor_clean(90000)

  print("======== Gravity SGX6410 runSensorClean ========")
  print("Init      : OK")
  if result == CLEAN_OK:
    print("Clean     : finished")
  elif result == CLEAN_DONE:
    print("Clean     : already cleaned, skip")
  elif result == CLEAN_TIMEOUT:
    print("Clean     : timeout, do not power off if still cleaning")
  else:
    print("Clean     : failed, check communication")
  print("Note      : Keep power stable. Clean takes about 60 s. Run only once.")
  print("==================================================")

def loop():
  # Clean runs only in setup()
  time.sleep(1)

if __name__ == "__main__":
  setup()
  while True:
    loop()
