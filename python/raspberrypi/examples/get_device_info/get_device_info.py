# -*- coding:utf-8 -*-
'''!
@file get_device_info.py
@brief 读取 SGX6410 的设备信息
@details 演示读取 VID、PID、固件版本、设备地址、SGX6410 产品 ID 和追踪号。
'''

import os
import sys
import time

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))))
from DFRobot_SGX6410_Gravity import *

# ctype=0 使用 I2C；ctype=1 使用 UART。
ctype = 0
ADDRESS = SGX_ADDR_DEFAULT
I2C_BUS = 1

if ctype == 0:
  sgx6410 = DFRobot_SGX6410_Gravity_I2C(I2C_BUS, ADDRESS)
else:
  sgx6410 = DFRobot_SGX6410_Gravity_UART(SGX_UART_BAUD_DEFAULT, ADDRESS)


def setup():
  # 初始化模块，并校验 DFRobot 厂商 ID（应为 0x3343）。
  while sgx6410.begin() == False:
    print("初始化失败，请检查地址、COM_SEL 和接线")
    time.sleep(1)

  print("======== SGX6410 设备信息 ========")
  # 读取 DFRobot 厂商 ID。
  print("厂商 VID       : 0x%04X" % sgx6410.get_vid())
  # 读取模块 PID，当前固件可能返回 0x0000。
  print("模块 PID       : 0x%04X" % sgx6410.get_pid())
  # 读取固件版本编码，例如 0x1000 表示 V1.0.0.0。
  print("固件版本       : 0x%04X" % sgx6410.get_fw_version())
  # 读取上电时锁存的 7 位设备地址，通常为 0x52 或 0x53。
  print("设备地址       : 0x%02X" % sgx6410.get_device_addr())
  # 读取 SGX6410 传感器产品 ID。
  print("SGX6410 产品ID : 0x%04X" % sgx6410.get_sgx_product_id())

  # 读取 6 字节追踪号；读取失败时返回 None。
  tracking = sgx6410.get_tracking_number()
  if tracking is None:
    print("追踪号         : 读取失败")
  else:
    print("追踪号         : " + " ".join("%02X" % value for value in tracking))
  print("===================================")


if __name__ == "__main__":
  setup()
