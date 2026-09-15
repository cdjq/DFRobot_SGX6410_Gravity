# -*- coding:utf-8 -*-
'''!
  @file run_sensor_clean.py
  @brief Run the Gravity SGX6410 thermal clean once
  @details Wait for command 'c' or 'C', then start the one-time thermal clean.
  @n After a successful clean the mode is Suspend. Do not send any further commands.
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

clean_started = False

def setup():
  # Initialize the module and verify VID 0x3343
  while (sgx6410.begin() == False):
    print("Init failed, check address, COM_SEL and wiring")
    time.sleep(1)

  print("======== Gravity SGX6410 runSensorClean ========")
  print("Init      : OK")
  print("Note      : Keep power stable. Clean takes about 60 s. Run only once.")
  print("Command   : Send c or C to start cleaning")
  print("==================================================")

def loop():
  global clean_started
  command = ""

  # After one clean attempt, do not send any further commands
  if clean_started == True:
    time.sleep(1)
    return
  command = sys.stdin.read(1)
  if (command != "c") and (command != "C"):
    return

  # About 60 s. Do not reset or remove power while cleaning
  clean_started = True
  print("Cleaning started; wait at least 60 seconds...")
  result = sgx6410.run_sensor_clean(90000)
  if result == CLEAN_OK:
    print("Cleaning completed successfully.")
  elif result == CLEAN_DONE:
    print("Cleaning was already performed.")
  elif result == CLEAN_TIMEOUT:
    print("Cleaning timed out; keep the sensor powered and inspect it.")
  else:
    print("Cleaning failed due to communication error.")

if __name__ == "__main__":
  setup()
  while True:
    loop()
