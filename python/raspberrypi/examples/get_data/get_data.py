# -*- coding:utf-8 -*-
'''!
  @file get_data.py
  @brief Read Gravity SGX6410 air quality data without humidity compensation
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

  # IAQ: about 3 s sample period, 5 min warm-up
  while (sgx6410.set_operation_mode(DFRobot_SGX6410_Gravity.IAQ) == False):
    print("Set IAQ mode failed")
    time.sleep(1)

  print("======== Gravity SGX6410 getData ========")
  print("Init      : OK")
  print("VID       : 0x%X"%(sgx6410.get_vid()))
  print("SGX PID   : 0x%X"%(sgx6410.get_sgx_product_id()))
  tracking = sgx6410.get_tracking_number()
  if tracking != None:
    text = ""
    i = 0
    while i < 6:
      if tracking[i] < 0x10:
        text = text + "0"
      text = text + ("%X"%(tracking[i]))
      if i < 5:
        text = text + " "
      i = i + 1
    print("Tracking  : " + text)
  else:
    print("Tracking  : --")
  print("Mode      : IAQ")
  print("==========================================")

def loop():
  # update() reads the measurement registers and scales them
  if (sgx6410.update() == True):
    data = sgx6410.get_all_gas_data()
    # Skip if the bridge has not published a new sample
    if data.is_new == False:
      time.sleep(0.5)
      return
    print("Humidity comp enable=%d"%(sgx6410.get_humidity_comp_enable()))
    print("Mode:     0x%X"%(sgx6410.get_operation_mode()))
    print("IAQ:      %.2f"%(data.iaq))
    print("TVOC:     %.2f mg/m3"%(data.tvoc))
    print("ETOH:     %.2f ppm"%(data.etoh))
    print("ECO2:     %.1f ppm"%(data.eco2))
    print("RELIAQ:   %.2f"%(data.reliaq))
    if data.valid == True:
      print("Data valid: YES")
    else:
      print("Data valid: NO (warming up)")
    print("--------------------")
  else:
    print("Failed to read gas data!")
  time.sleep(1)

if __name__ == "__main__":
  setup()
  while True:
    loop()
