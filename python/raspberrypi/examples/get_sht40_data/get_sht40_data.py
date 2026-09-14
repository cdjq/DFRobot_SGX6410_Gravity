# -*- coding:utf-8 -*-
'''!
  @file get_sht40_data.py
  @brief Read on-board SHT40 temperature and humidity
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
import math

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

  # Firmware reads SHT40 only after automatic compensation is enabled
  while (sgx6410.set_auto_humi_compensation(HUM_COMP_ENABLE) == False):
    print("Enable humidity compensation failed")
    time.sleep(1)

  print("======== Gravity SGX6410 getSHT40Data ========")
  print("Init      : OK")
  print("HumComp   : ON")
  print("===============================================")

def loop():
  # Temperature = -45 + 175 * raw / 65535, humidity = -6 + 125 * raw / 65535
  sht40 = sgx6410.get_sht40_data()
  if (math.isnan(sht40.temperature) == True) or (math.isnan(sht40.humidity) == True):
    print("SHT40 data not ready, wait for humidity compensation")
  else:
    print("Temp:     %.2f C"%(sht40.temperature))
    print("Humidity: %.2f %%RH"%(sht40.humidity))
    print("--------------------")
  time.sleep(1)

if __name__ == "__main__":
  setup()
  while True:
    loop()
