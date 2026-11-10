# -*- coding:utf-8 -*-

'''
@file DFRobot_RTU.py
@brief Modbus RTU libary for Arduino.

@copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
@licence     The MIT License (MIT)
@author [Arya](xue.peng@dfrobot.com)
@version  V1.0
@date  2021-07-16
@https://github.com/DFRobot/DFRobot_RTU
'''

import serial
import time


class DFRobot_RTU(object):
  _packet_header = {"id": 0, "cmd": 1, "cs": 0}

  '''Enum constant'''
  eRTU_EXCEPTION_ILLEGAL_FUNCTION = 0x01
  eRTU_EXCEPTION_ILLEGAL_DATA_ADDRESS = 0x02
  eRTU_EXCEPTION_ILLEGAL_DATA_VALUE = 0x03
  eRTU_EXCEPTION_SLAVE_FAILURE = 0x04
  eRTU_EXCEPTION_CRC_ERROR = 0x08
  eRTU_RECV_ERROR = 0x09
  eRTU_MEMORY_ERROR = 0x0A
  eRTU_ID_ERROR = 0x0B

  eCMD_READ_COILS = 0x01
  eCMD_READ_DISCRETE = 0x02
  eCMD_READ_HOLDING = 0x03
  eCMD_READ_INPUT = 0x04
  eCMD_WRITE_COILS = 0x05
  eCMD_WRITE_HOLDING = 0x06
  eCMD_WRITE_MULTI_COILS = 0x0F
  eCMD_WRITE_MULTI_HOLDING = 0x10

  def __init__(self, baud, bits, parity, stopbit):
    '''
    @brief Serial initialization.
    @param baud:  The UART baudrate of raspberry pi
    @param bits:  The UART data bits of raspberry pi
    @param parity:  The UART parity bits of raspberry pi
    @param stopbit:  The UART stopbit bits of raspberry pi.
    '''
    self._ser = serial.Serial("/dev/ttyAMA0", baud, bits, parity, stopbit)
    self._timeout = 0.1  # 0.1s

  def set_timout_time_s(self, timeout=0.1):
    '''
    @brief Set receive timeout time, unit s.
    @param timeout:  receive timeout time, unit s, default 0.1s.
    '''
    self._timeout = timeout

  def read_coils_register(self, device_addr, reg):
    '''
    @brief Read a coils Register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Coils register address.
    @return Return the value of the coils register value.
    @n      True: The value of the coils register value is 1.
    @n      False: The value of the coils register value is 0.
    '''
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), 0x00, 0x01]
    val = False
    if (device_addr < 1) or (device_addr > 0xF7):
      print("device addr error.(1~247) %d" % device_addr)
      return 0
    packet = self._packed(device_addr, self.eCMD_READ_COILS, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_READ_COILS, 1)
    if (packet[0] == 0) and len(packet) == 7:
      if (packet[4] & 0x01) != 0:
        val = True
    return val

  def read_discrete_inputs_register(self, device_addr, reg):
    '''
    @brief Read a discrete input register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Discrete input register address.
    @return Return the value of the discrete input register value.
    @n      True: The value of the discrete input register value is 1.
    @n      False: The value of the discrete input register value is 0.
    '''
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), 0x00, 0x01]
    val = False
    if (device_addr < 1) or (device_addr > 0xF7):
      print("device addr error.(1~247) %d" % device_addr)
      return 0
    packet = self._packed(device_addr, self.eCMD_READ_DISCRETE, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_READ_DISCRETE, 1)
    if (packet[0] == 0) and len(packet) == 7:
      if (packet[4] & 0x01) != 0:
        val = True
    return val

  def read_holding_register(self, device_addr, reg):
    '''
    @brief Read a holding Register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Holding register address.
    @return Return the value of the holding register value.
    '''
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), 0x00, 0x01]
    if (device_addr < 1) or (device_addr > 0xF7):
      print("device addr error.(1~247) %d" % device_addr)
      return 0
    packet = self._packed(device_addr, self.eCMD_READ_HOLDING, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_READ_HOLDING, 2)
    if (packet[0] == 0) and len(packet) == 8:
      packet[0] = ((packet[4] << 8) | packet[5]) & 0xFFFF
    else:
      packet[0] = 0
    return packet[0]

  def read_input_register(self, device_addr, reg):
    '''
    @brief Read a input Register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Input register address.
    @return Return the value of the holding register value.
    '''
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), 0x00, 0x01]
    if (device_addr < 1) or (device_addr > 0xF7):
      print("device addr error.(1~247) %d" % device_addr)
      return 0
    packet = self._packed(device_addr, self.eCMD_READ_INPUT, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_READ_INPUT, 2)
    if (packet[0] == 0) and len(packet) == 8:
      packet[0] = ((packet[4] << 8) | packet[5]) & 0xFFFF
    else:
      packet[0] = 0
    return packet[0]

  def write_coils_register(self, device_addr, reg, flag):
    '''
    @brief Write a coils Register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Coils register address.
    @param flag: The value of the register value which will be write, 0 ro 1.
    @return Exception code:
    @n      0 : sucess.
    @n      1 or eRTU_EXCEPTION_ILLEGAL_FUNCTION : Illegal function.
    @n      2 or eRTU_EXCEPTION_ILLEGAL_DATA_ADDRESS: Illegal data address.
    @n      3 or eRTU_EXCEPTION_ILLEGAL_DATA_VALUE:  Illegal data value.
    @n      4 or eRTU_EXCEPTION_SLAVE_FAILURE:  Slave failure.
    @n      8 or eRTU_EXCEPTION_CRC_ERROR:  CRC check error.
    @n      9 or eRTU_RECV_ERROR:  Receive packet error.
    @n      10 or eRTU_MEMORY_ERROR: Memory error.
    @n      11 or eRTU_ID_ERROR: Broadcasr address or error ID
    '''
    val = 0x0000
    if flag:
      val = 0xFF00
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), (val >> 8) & 0xFF, (val & 0xFF)]
    if device_addr > 0xF7:
      print("device addr error.")
      return 0
    packet = self._packed(device_addr, self.eCMD_WRITE_COILS, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_WRITE_COILS, reg)
    return packet[0]

  def write_holding_register(self, device_addr, reg, val):
    '''
    @brief Write a holding register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Holding register address.
    @param val: The value of the register value which will be write.
    @return Exception code:
    @n      0 : sucess.
    @n      1 or eRTU_EXCEPTION_ILLEGAL_FUNCTION : Illegal function.
    @n      2 or eRTU_EXCEPTION_ILLEGAL_DATA_ADDRESS: Illegal data address.
    @n      3 or eRTU_EXCEPTION_ILLEGAL_DATA_VALUE:  Illegal data value.
    @n      4 or eRTU_EXCEPTION_SLAVE_FAILURE:  Slave failure.
    @n      8 or eRTU_EXCEPTION_CRC_ERROR:  CRC check error.
    @n      9 or eRTU_RECV_ERROR:  Receive packet error.
    @n      10 or eRTU_MEMORY_ERROR: Memory error.
    @n      11 or eRTU_ID_ERROR: Broadcasr address or error ID
    '''
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), (val >> 8) & 0xFF, (val & 0xFF)]
    val = 0
    if device_addr > 0xF7:
      print("device addr error.")
      return 0
    packet = self._packed(device_addr, self.eCMD_WRITE_HOLDING, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_WRITE_HOLDING, reg)
    return packet[0]

  def read_coils_registers(self, device_addr, reg, reg_num):
    '''
    @brief Read multiple coils Register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Read the start address of the coil register.
    @param reg_num: Number of coils Register.
    @return list: format as follow:
    @n      list[0]: Exception code:
    @n               0 : sucess.
    @n               1 or eRTU_EXCEPTION_ILLEGAL_FUNCTION : Illegal function.
    @n               2 or eRTU_EXCEPTION_ILLEGAL_DATA_ADDRESS: Illegal data address.
    @n               3 or eRTU_EXCEPTION_ILLEGAL_DATA_VALUE:  Illegal data value.
    @n               4 or eRTU_EXCEPTION_SLAVE_FAILURE:  Slave failure.
    @n               8 or eRTU_EXCEPTION_CRC_ERROR:  CRC check error.
    @n               9 or eRTU_RECV_ERROR:  Receive packet error.
    @n               10 or eRTU_MEMORY_ERROR: Memory error.
    @n               11 or eRTU_ID_ERROR: Broadcasr address or error ID
    @n      list[1:]: The value of the coil register list.
    '''
    length = reg_num // 8
    mod = reg_num % 8
    if mod:
      length += 1
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), (reg_num >> 8) & 0xFF, reg_num & 0xFF]
    if (device_addr < 1) or (device_addr > 0xF7):
      print("device addr error.(1~247) %d" % device_addr)
      return [self.eRTU_ID_ERROR]
    packet = self._packed(device_addr, self.eCMD_READ_COILS, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_READ_COILS, length)
    if (packet[0] == 0) and (len(packet) == (5 + length + 1)):
      values = [packet[0]] + packet[4 : len(packet) - 2]
      return values
    return [packet[0]]

  def read_discrete_inputs_registers(self, device_addr, reg, reg_num):
    '''
    @brief Read multiple discrete inputs register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Read the start address of the discrete inputs register.
    @param reg_num: Number of coils Register.
    @return list: format as follow:
    @n      list[0]: Exception code:
    @n               0 : sucess.
    @n               1 or eRTU_EXCEPTION_ILLEGAL_FUNCTION : Illegal function.
    @n               2 or eRTU_EXCEPTION_ILLEGAL_DATA_ADDRESS: Illegal data address.
    @n               3 or eRTU_EXCEPTION_ILLEGAL_DATA_VALUE:  Illegal data value.
    @n               4 or eRTU_EXCEPTION_SLAVE_FAILURE:  Slave failure.
    @n               8 or eRTU_EXCEPTION_CRC_ERROR:  CRC check error.
    @n               9 or eRTU_RECV_ERROR:  Receive packet error.
    @n               10 or eRTU_MEMORY_ERROR: Memory error.
    @n               11 or eRTU_ID_ERROR: Broadcasr address or error ID
    @n      list[1:]: The value list of the discrete inputs register.
    '''
    length = reg_num // 8
    mod = reg_num % 8
    if mod:
      length += 1
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), (reg_num >> 8) & 0xFF, reg_num & 0xFF]
    if (device_addr < 1) or (device_addr > 0xF7):
      print("device addr error.(1~247) %d" % device_addr)
      return [self.eRTU_ID_ERROR]
    packet = self._packed(device_addr, self.eCMD_READ_DISCRETE, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_READ_DISCRETE, length)
    if (packet[0] == 0) and (len(packet) == (5 + length + 1)):
      values = [packet[0]] + packet[4 : len(packet) - 2]
      return values
    return [packet[0]]

  def read_holding_registers(self, device_addr, reg, size):
    '''
    @brief Read multiple holding register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Read the start address of the holding register.
    @param size: Number of read holding register.
    @return list: format as follow:
    @n      list[0]: Exception code:
    @n               0 : sucess.
    @n               1 or eRTU_EXCEPTION_ILLEGAL_FUNCTION : Illegal function.
    @n               2 or eRTU_EXCEPTION_ILLEGAL_DATA_ADDRESS: Illegal data address.
    @n               3 or eRTU_EXCEPTION_ILLEGAL_DATA_VALUE:  Illegal data value.
    @n               4 or eRTU_EXCEPTION_SLAVE_FAILURE:  Slave failure.
    @n               8 or eRTU_EXCEPTION_CRC_ERROR:  CRC check error.
    @n               9 or eRTU_RECV_ERROR:  Receive packet error.
    @n               10 or eRTU_MEMORY_ERROR: Memory error.
    @n               11 or eRTU_ID_ERROR: Broadcasr address or error ID
    @n      list[1:]: The value list of the holding register.
    '''
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), (size >> 8) & 0xFF, size & 0xFF]
    if (device_addr < 1) or (device_addr > 0xF7):
      print("device addr error.(1~247) %d" % device_addr)
      return [self.eRTU_ID_ERROR]
    packet = self._packed(device_addr, self.eCMD_READ_HOLDING, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_READ_HOLDING, size * 2)
    # lin = ['%02X' % i for i in l]
    # print(" ".join(lin))
    if (packet[0] == 0) and (len(packet) == (5 + size * 2 + 1)):
      values = [packet[0]] + packet[4 : len(packet) - 2]
      return values
    return [packet[0]]

  def read_input_registers(self, device_addr, reg, size):
    '''
    @brief Read multiple input register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Read the start address of the input register.
    @param size: Number of read input register.
    @return list: format as follow:
    @n      list[0]: Exception code:
    @n               0 : sucess.
    @n               1 or eRTU_EXCEPTION_ILLEGAL_FUNCTION : Illegal function.
    @n               2 or eRTU_EXCEPTION_ILLEGAL_DATA_ADDRESS: Illegal data address.
    @n               3 or eRTU_EXCEPTION_ILLEGAL_DATA_VALUE:  Illegal data value.
    @n               4 or eRTU_EXCEPTION_SLAVE_FAILURE:  Slave failure.
    @n               8 or eRTU_EXCEPTION_CRC_ERROR:  CRC check error.
    @n               9 or eRTU_RECV_ERROR:  Receive packet error.
    @n               10 or eRTU_MEMORY_ERROR: Memory error.
    @n               11 or eRTU_ID_ERROR: Broadcasr address or error ID
    @n      list[1:]: The value list of the input register.
    '''
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), (size >> 8) & 0xFF, size & 0xFF]
    if (device_addr < 1) or (device_addr > 0xF7):
      print("device addr error.(1~247) %d" % device_addr)
      return [self.eRTU_ID_ERROR]
    packet = self._packed(device_addr, self.eCMD_READ_INPUT, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_READ_INPUT, size * 2)
    # lin = ['%02X' % i for i in l]
    # print(" ".join(lin))
    if (packet[0] == 0) and (len(packet) == (5 + size * 2 + 1)):
      values = [packet[0]] + packet[4 : len(packet) - 2]
      return values
    return [packet[0]]

  def write_coils_registers(self, device_addr, reg, reg_num, data):
    '''
    @brief Write multiple coils Register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Write the start address of the coils register.
    @param reg_num: Number of coils Register.
    @param data: The list of storage coils Registers' value which will be write.
    @return Exception code:
    @n      0 : sucess.
    @n      1 or eRTU_EXCEPTION_ILLEGAL_FUNCTION : Illegal function.
    @n      2 or eRTU_EXCEPTION_ILLEGAL_DATA_ADDRESS: Illegal data address.
    @n      3 or eRTU_EXCEPTION_ILLEGAL_DATA_VALUE:  Illegal data value.
    @n      4 or eRTU_EXCEPTION_SLAVE_FAILURE:  Slave failure.
    @n      8 or eRTU_EXCEPTION_CRC_ERROR:  CRC check error.
    @n      9 or eRTU_RECV_ERROR:  Receive packet error.
    @n      10 or eRTU_MEMORY_ERROR: Memory error.
    @n      11 or eRTU_ID_ERROR: Broadcasr address or error ID
    '''
    length = reg_num // 8
    mod = reg_num % 8
    if mod:
      length += 1
    if len(data) < length:
      return [self.eRTU_EXCEPTION_ILLEGAL_DATA_VALUE]
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), ((reg_num >> 8) & 0xFF), (reg_num & 0xFF), length] + data
    if device_addr > 0xF7:
      print("device addr error.")
      return 0
    packet = self._packed(device_addr, self.eCMD_WRITE_MULTI_COILS, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_WRITE_MULTI_COILS, reg)
    return packet[0]

  def write_holding_registers(self, device_addr, reg, data):
    '''
    @brief Write multiple holding Register.
    @param device_addr:  modbus device ID. Range: 0x00 ~ 0xF7(0~247), 0x00 is broadcasr address, which all slaves will process broadcast packets,
    @n          but will not answer.
    @param reg: Write the start address of the holding register.
    @param data: The list of storage holding Registers' value which will be write.
    @return Exception code:
    @n      0 : sucess.
    @n      1 or eRTU_EXCEPTION_ILLEGAL_FUNCTION : Illegal function.
    @n      2 or eRTU_EXCEPTION_ILLEGAL_DATA_ADDRESS: Illegal data address.
    @n      3 or eRTU_EXCEPTION_ILLEGAL_DATA_VALUE:  Illegal data value.
    @n      4 or eRTU_EXCEPTION_SLAVE_FAILURE:  Slave failure.
    @n      8 or eRTU_EXCEPTION_CRC_ERROR:  CRC check error.
    @n      9 or eRTU_RECV_ERROR:  Receive packet error.
    @n      10 or eRTU_MEMORY_ERROR: Memory error.
    @n      11 or eRTU_ID_ERROR: Broadcasr address or error ID
    '''
    size = len(data) >> 1
    packet = [(reg >> 8) & 0xFF, (reg & 0xFF), ((size >> 8) & 0xFF), (size & 0xFF), size * 2] + data
    if device_addr > 0xF7:
      print("device addr error.")
      return 0
    packet = self._packed(device_addr, self.eCMD_WRITE_MULTI_HOLDING, packet)
    self._send_package(packet)
    packet = self.recv_and_parse_package(device_addr, self.eCMD_WRITE_MULTI_HOLDING, reg)
    return packet[0]

  def _calculate_crc(self, data):
    crc = 0xFFFF
    length = len(data)
    # print("len=%d"%length)
    pos = 0
    while pos < length:
      crc ^= data[pos] | 0x0000
      # print("pos=%d, %02x"%(pos,data[pos]))
      i = 8
      while i != 0:
        if (crc & 0x0001) != 0:
          crc = (crc >> 1) & 0xFFFF
          crc ^= 0xA001
        else:
          crc = (crc >> 1) & 0xFFFF
        i -= 1
      pos += 1
    crc = (((crc & 0x00FF) << 8) | ((crc & 0xFF00) >> 8)) & 0xFFFF
    # print("crc=%x"%crc)
    return crc

  def _clear_recv_buffer(self):
    remain = self._ser.inWaiting()
    while remain:
      self._ser.read(remain)
      remain = self._ser.inWaiting()

  def _packed(self, device_addr, cmd, packet):
    length = 4 + len(packet)
    # print(len(l))
    package = [0] * length
    package[0] = device_addr
    package[1] = cmd
    package[2 : length - 2] = packet
    # lin = ['%02X' % i for i in package]
    # print(" ".join(lin))

    crc = self._calculate_crc(package[: len(package) - 2])
    package[length - 2] = (crc >> 8) & 0xFF
    package[length - 1] = crc & 0xFF

    # lin = ['%02X' % i for i in package]
    # print(" ".join(lin))
    return package

  def _send_package(self, packet):
    self._clear_recv_buffer()
    if len(packet):
      self._ser.write(packet)
      time.sleep(self._timeout)

  def recv_and_parse_package(self, device_addr, cmd, val):
    package = [self.eRTU_ID_ERROR]
    if device_addr == 0:
      return [0]
    if (device_addr < 1) or (device_addr > 0xF7):
      return package
    head = [0] * 4
    index = 0
    last_received = time.time()
    remain = 0
    while remain < 4:
      if self._ser.inWaiting():
        data = self._ser.read(1)
        try:
          head[index] = ord(data)
        except:
          head[index] = data
        # print("%d = %02X"%(index, head[index]))
        index += 1
        if (index == 1) and (head[0] != device_addr):
          index = 0
        elif (index == 2) and ((head[1] & 0x7F) != cmd):
          index = 0
        remain = index
        last_received = time.time()
      if time.time() - last_received > self._timeout:
        # print("time out.")
        return [self.eRTU_RECV_ERROR]
      if index == 4:
        if head[1] & 0x80:
          index = 5
        else:
          if head[1] < 5:
            if head[2] != (val & 0xFF):
              index = 0
              remain = 0
            else:
              index = 5 + head[2]
          else:
            if (((head[2] << 8) | head[3]) & 0xFFFF) != val:
              index = 0
              remain = 0
            else:
              index = 8
        if index > 0:
          package = [0] * (index + 1)
          package[1:5] = head
          remain = index - 4
          index = 5
          last_received = time.time()
          while remain > 0:
            if self._ser.inWaiting():
              data = self._ser.read(1)
              try:
                package[index] = ord(data)
              except:
                package[index] = data
              index += 1
              remain -= 1
              last_received = time.time()
            if time.time() - last_received > self._timeout:
              print("time out1.")
              return [self.eRTU_RECV_ERROR]
          crc = ((package[len(package) - 2] << 8) | package[len(package) - 1]) & 0xFFFF
          if crc != self._calculate_crc(package[1 : len(package) - 2]):
            print("CRC ERROR")
            return [self.eRTU_RECV_ERROR]
          if package[2] & 0x80:
            package[0] = package[3]
          else:
            package[0] = 0
          # lin = ['%02X' % i for i in package]
          # print(" ".join(lin))
          return package
