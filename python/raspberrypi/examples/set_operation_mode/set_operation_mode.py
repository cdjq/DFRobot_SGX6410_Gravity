# -*- coding:utf-8 -*-
'''!
@file set_operation_mode.py
@brief Set and read the Gravity SGX6410 operation mode
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

# Select operation mode: IAQ / ULP / PBAQ / SUSPEND
sgx6410_operation_mode = DFRobot_SGX6410_Gravity.PBAQ

ctype = 0
address = SGX_ADDR_DEFAULT
i2c_1 = 0x01

sgx6410: DFRobot_SGX6410_Gravity
if ctype == 0:
  sgx6410 = DFRobot_SGX6410_Gravity_I2C(i2c_1, address)
else:
  sgx6410 = DFRobot_SGX6410_Gravity_UART(SGX_UART_BAUD_DEFAULT, address)


def setup():
  # Initialize the module and verify VID 0x3343
  while sgx6410.begin() == False:
    print("Init failed, check address, COM_SEL and wiring")
    time.sleep(1)

  # Apply the operation mode selected above
  while sgx6410.set_operation_mode(sgx6410_operation_mode) == False:
    print("Set operation mode failed")
    time.sleep(1)

  print("======== Gravity SGX6410 setOperationMode ========")
  print("Init      : OK")
  print("Mode      : 0x%X" % (sgx6410.get_operation_mode()))
  print("===================================================")


def loop():
  # update() reads the measurement registers and scales them
  if sgx6410.update() == True:
    data = sgx6410.get_all_gas_data()
    if data.is_new == False:
      time.sleep(0.5)
      return
    print("Mode:     0x%X" % (sgx6410.get_operation_mode()))
    print("IAQ:      %.2f" % (data.iaq))
    print("TVOC:     %.2f mg/m3" % (data.tvoc))
    print("ETOH:     %.2f ppm" % (data.etoh))
    print("ECO2:     %.1f ppm" % (data.eco2))
    print("RELIAQ:   %.2f" % (data.reliaq))
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
