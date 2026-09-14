# -*- coding:utf-8 -*-
'''!
  @file DFRobot_SGX6410_Gravity.py
  @brief Gravity MEMS Air Quality Sensor (SGX6410) Raspberry Pi library
  @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license The MIT License (MIT)
  @author PLELES(li.jia@dfrobot.com)
  @version V1.0.0
  @date 2026-09-12
  @url https://github.com/DFRobot/DFRobot_SGX6410_Gravity
'''
import math
import time
import ctypes
try:
  import smbus
except:
  import smbus2 as smbus

from DFRobot_RTU import DFRobot_RTU

I2C_MODE  = 0x01
UART_MODE = 0x02

SGX_ADDR_LOW          = 0x52
SGX_ADDR_HIGH         = 0x53
SGX_ADDR_DEFAULT      = 0x53
SGX_UART_BAUD_DEFAULT = 9600
DEVICE_VID            = 0x3343
DEVICE_PID            = 0x0000
DEVICE_FW_VERSION     = 0x1000
SGX_PRODUCT_ID        = 0x2310

HUM_COMP_DISABLE   = 0x0000
HUM_COMP_ENABLE    = 0x0001
SENSOR_RESET_VALUE = 0x0001

SGX_WARMUP_IAQ_MS  = 300000
SGX_WARMUP_ULP_MS  = 900000
SGX_WARMUP_PBAQ_MS = 300000
SGX_POLL_IAQ_MS    = 3000
SGX_POLL_ULP_MS    = 90000
SGX_POLL_PBAQ_MS   = 5000

REG_I_PID           = 0x0000
REG_I_VID           = 0x0001
REG_I_DEVICE_ADDR   = 0x0002
REG_I_RESERVED0     = 0x0003
REG_I_RESERVED1     = 0x0004
REG_I_VERSION       = 0x0005
REG_I_SGX6410_PID   = 0x0006
REG_I_TRACKING_NUM_0 = 0x0007
REG_I_TRACKING_NUM_1 = 0x0008
REG_I_TRACKING_NUM_2 = 0x0009
REG_I_IAQ           = 0x000A
REG_I_TVOC          = 0x000B
REG_I_ETOH          = 0x000C
REG_I_ECO2          = 0x000D
REG_I_RELIAQ        = 0x000E
REG_I_DATA_IS_NEW   = 0x000F
REG_I_DATA_VALID    = 0x0010
REG_I_SENSORS_MODE  = 0x0011
REG_I_TEMP_RAW      = 0x0012
REG_I_HUM_RAW       = 0x0013

REG_H_RESERVED0       = 0x0000
REG_H_RESERVED1       = 0x0001
REG_H_RESERVED2       = 0x0002
REG_H_BAUDRATE        = 0x0003
REG_H_VERIFY_AND_STOP = 0x0004
REG_H_RESERVED3       = 0x0005
REG_H_SENSORS_MODE    = 0x0006
REG_H_SENSORS_RESET   = 0x0007
REG_H_HUMIDITY        = 0x0008
REG_H_HUM_COMP_EN     = 0x0009
REG_H_CLEAN_FLAG      = 0x000A

REG_I2C_VID            = 0x0000
REG_I2C_PID            = 0x0001
REG_I2C_FW_VERSION     = 0x0002
REG_I2C_DEVICE_ADDR    = 0x0003
REG_I2C_SENSORS_RESET  = 0x0004
REG_I2C_SENSORS_MODE   = 0x0005
REG_I2C_HUMIDITY       = 0x0006
REG_I2C_HUM_COMP_EN    = 0x0007
REG_I2C_SGX6410_PID    = 0x0008
REG_I2C_TRACKING_NUM_0 = 0x0009
REG_I2C_TRACKING_NUM_1 = 0x000A
REG_I2C_TRACKING_NUM_2 = 0x000B
REG_I2C_IAQ            = 0x000C
REG_I2C_TVOC           = 0x000D
REG_I2C_ETOH           = 0x000E
REG_I2C_ECO2           = 0x000F
REG_I2C_RELIAQ         = 0x0010
REG_I2C_DATA_IS_NEW    = 0x0011
REG_I2C_DATA_VALID     = 0x0012
REG_I2C_TEMP_RAW       = 0x0013
REG_I2C_HUM_RAW        = 0x0014
REG_I2C_CLEAN_FLAG     = 0x0015

RET_CODE_OK    = 0
RET_CODE_ERROR = 1
CLEAN_OK       = 0
CLEAN_DONE     = 1
CLEAN_TIMEOUT  = 2
CLEAN_FAIL     = 3

class GasData(ctypes.Structure):
  '''!
    @brief Parsed and cached gas measurement data
  '''
  _fields_ = [
    ("iaq",     ctypes.c_float),
    ("tvoc",    ctypes.c_float),
    ("etoh",    ctypes.c_float),
    ("eco2",    ctypes.c_float),
    ("reliaq",  ctypes.c_float),
    ("is_new",  ctypes.c_bool),
    ("valid",   ctypes.c_bool),
  ]

class SHT40Data(ctypes.Structure):
  '''!
    @brief On-board SHT40 temperature and humidity
  '''
  _fields_ = [
    ("temperature", ctypes.c_float),
    ("humidity",    ctypes.c_float),
  ]

class DFRobot_SGX6410_Gravity(object):
  COMM_MODE_UART = 0
  COMM_MODE_I2C  = 1
  INPUT_REG      = 0
  HOLDING_REG    = 1
  SUSPEND        = 0x00
  IAQ            = 0x01
  ULP            = 0x02
  PBAQ           = 0x05
  SENSOR_CLEAN   = 0x80
  DATA_IAQ       = 0
  DATA_TVOC      = 1
  DATA_ETOH      = 2
  DATA_ECO2      = 3
  DATA_RELIAQ    = 4
  HUM_COMP_DISABLE = 0x0000
  HUM_COMP_ENABLE  = 0x0001

  def __init__(self, bus=1, baud=9600, mode=I2C_MODE):
    '''!
      @brief Constructor
      @param bus I2C bus number
      @param baud UART baud rate
      @param mode I2C_MODE or UART_MODE
    '''
    self._mode = self.SUSPEND
    self._gas_data = GasData(float('nan'), float('nan'), float('nan'), float('nan'), float('nan'), False, False)
    self._sht40_data = SHT40Data(float('nan'), float('nan'))
    if mode == I2C_MODE:
      self.i2cbus = smbus.SMBus(bus)
      self._uart_i2c = I2C_MODE
    else:
      self._uart_i2c = UART_MODE

  def begin(self):
    '''!
      @brief Initialize the module and verify the vendor ID
      @return True on success, False on failure
      @n Reads DEVICE_VID (0x3343). Module PID may still be 0x0000 until the product ID is assigned.
    '''
    vid = self.read_u16(REG_I_VID, REG_I2C_VID, self.INPUT_REG)
    if vid is None:
      return False
    if vid != DEVICE_VID:
      return False
    self._mode = self.get_operation_mode()
    return True

  def reset(self):
    '''!
      @brief Restore sensor reset command on the bridge
      @return True on success, False on failure
      @n Host writes 0x0001 to the reset register. The register reads back as 0.
      @n After reset, wait for the module to finish SGX6410 start-up before calling update().
    '''
    if self.write_u16(REG_H_SENSORS_RESET, REG_I2C_SENSORS_RESET, SENSOR_RESET_VALUE) == False:
      return False
    time.sleep(0.05)
    self._gas_data.is_new = False
    self._gas_data.valid = False
    return True

  def set_operation_mode(self, mode):
    '''!
      @brief Set SGX6410 operation mode
      @param mode Operation mode
      @n SUSPEND: Suspend mode
      @n IAQ: IAQ 2nd generation, about 3 s sample period, 5 min warm-up
      @n ULP: Ultra-low power, about 90 s sample period, 15 min warm-up
      @n PBAQ: PBAQ, about 5 s sample period, 5 min warm-up
      @n Mode change is a maintenance action. The bridge restarts warm-up and clears is_new/valid.
      @n Sensor cleaning 0x80 should be started with run_sensor_clean(), not called repeatedly.
      @return True on success, False on failure
    '''
    if mode not in (self.SUSPEND, self.IAQ, self.ULP, self.PBAQ, self.SENSOR_CLEAN):
      return False
    if self.write_u16(REG_H_SENSORS_MODE, REG_I2C_SENSORS_MODE, mode) == False:
      return False
    time.sleep(0.05)
    self._mode = mode
    self._gas_data.is_new = False
    self._gas_data.valid = False
    return True

  def run_sensor_clean(self, timeout_ms):
    '''!
      @brief Run the sensor thermal-clean sequence: start clean, then poll until done
      @param timeout_ms Timeout in ms, recommend >= 90000 (clean takes about 60 s)
      @return CLEAN_OK=0 finished / CLEAN_DONE=1 already cleaned / CLEAN_TIMEOUT=2 timeout / CLEAN_FAIL=3 comm error
      @n The sensor should be cleaned only once in its lifetime.
      @n Reads the clean flag first. If it is 1, returns CLEAN_DONE without sending 0x80.
      @n Otherwise writes SENSOR_CLEAN (0x80) and polls the mode register: 0x80 still cleaning, 0x00 finished.
    '''
    flag = self.read_u16(REG_H_CLEAN_FLAG, REG_I2C_CLEAN_FLAG, self.HOLDING_REG)
    if flag is None:
      return CLEAN_FAIL
    if (flag & 0xFF) == 1:
      return CLEAN_DONE
    if self.set_operation_mode(self.SENSOR_CLEAN) == False:
      return CLEAN_FAIL
    start = int(time.time() * 1000)
    while (int(time.time() * 1000) - start) < timeout_ms:
      time.sleep(1)
      mode = self.read_u16(REG_I_SENSORS_MODE, REG_I2C_SENSORS_MODE, self.INPUT_REG)
      if mode is None:
        return CLEAN_FAIL
      mode = mode & 0xFF
      if mode == self.SUSPEND:
        self._mode = self.SUSPEND
        return CLEAN_OK
    return CLEAN_TIMEOUT

  def get_operation_mode(self):
    '''!
      @brief Read the current SGX6410 operation mode
      @return Current mode register value
      @n Returns SUSPEND when the read fails.
    '''
    value = self.read_u16(REG_I_SENSORS_MODE, REG_I2C_SENSORS_MODE, self.INPUT_REG)
    if value is None:
      return self.SUSPEND
    self._mode = value & 0xFF
    return self._mode

  def set_humidity_compensate(self, rh):
    '''!
      @brief Write ambient relative humidity for algorithm compensation
      @param rh Relative humidity, range 0.0 to 100.0, unit %RH
      @n Host converts rh to code = round(rh * 255 / 100) and writes the 0-255 code.
      @n This is a manual write. It does not enable the on-board SHT40 path.
      @n If automatic compensation is on, the bridge may overwrite this value on the next SHT40 cycle.
      @return True on success, False on failure
    '''
    if (math.isnan(rh) == True) or (rh < 0.0) or (rh > 100.0):
      return False
    code = int((rh * 255.0 / 100.0) + 0.5) & 0xFF
    if self.write_u16(REG_H_HUMIDITY, REG_I2C_HUMIDITY, code) == False:
      return False
    time.sleep(0.05)
    return True

  def get_humidity_compensate(self):
    '''!
      @brief Read the last humidity code as %RH
      @return Relative humidity in %RH reconstructed from the 0-255 code
      @n Returns NAN when the register read fails.
      @n This is the last value written to SGX6410, not a live SHT40 sample.
    '''
    raw = self.read_u16(REG_H_HUMIDITY, REG_I2C_HUMIDITY, self.HOLDING_REG)
    if raw is None:
      return float('nan')
    return ((raw & 0xFF) * 100.0) / 255.0

  def set_auto_humi_compensation(self, enable):
    '''!
      @brief Enable or disable automatic SHT40 humidity compensation on the bridge
      @param enable Compensation switch
      @n HUM_COMP_DISABLE: stop SHT40 reads; SGX6410 keeps the last humidity code
      @n HUM_COMP_ENABLE: read SHT40 immediately, then about every 60 s
      @n Turning off compensation does not restore the SGX6410 default 50 %RH unless you write it with set_humidity_compensate().
      @return True on success, False on failure
    '''
    if (enable != HUM_COMP_DISABLE) and (enable != HUM_COMP_ENABLE):
      return False
    if self.write_u16(REG_H_HUM_COMP_EN, REG_I2C_HUM_COMP_EN, enable) == False:
      return False
    time.sleep(0.05)
    return True

  def get_humidity_comp_enable(self):
    '''!
      @brief Read the automatic humidity compensation switch
      @return Current switch
      @n Returns HUM_COMP_DISABLE when the read fails.
    '''
    raw = self.read_u16(REG_H_HUM_COMP_EN, REG_I2C_HUM_COMP_EN, self.HOLDING_REG)
    if raw is None:
      return HUM_COMP_DISABLE
    if raw == HUM_COMP_ENABLE:
      return HUM_COMP_ENABLE
    return HUM_COMP_DISABLE

  def get_vid(self):
    '''!
      @brief Read DFRobot vendor ID
      @return Vendor ID, expected 0x3343
      @n Returns 0 when the read fails.
    '''
    value = self.read_u16(REG_I_VID, REG_I2C_VID, self.INPUT_REG)
    if value is None:
      return 0
    return value

  def get_pid(self):
    '''!
      @brief Read module product ID
      @return Module PID
      @n Firmware currently reports 0x0000 until the product ID is assigned.
      @n Returns 0 when the read fails.
    '''
    value = self.read_u16(REG_I_PID, REG_I2C_PID, self.INPUT_REG)
    if value is None:
      return 0
    return value

  def get_fw_version(self):
    '''!
      @brief Read module firmware version
      @return Encoded version, V1.0.0.0 is 0x1000
      @n Returns 0 when the read fails.
    '''
    value = self.read_u16(REG_I_VERSION, REG_I2C_FW_VERSION, self.INPUT_REG)
    if value is None:
      return 0
    return value

  def get_device_addr(self):
    '''!
      @brief Read the latched external slave address
      @return 7-bit address, 0x52 or 0x53
      @n DIP ADD_SEL is sampled at power-up and is also the Modbus slave address in UART mode.
      @n Changing the switch at run time does not take effect until reset.
      @n Returns 0 when the read fails.
    '''
    value = self.read_u16(REG_I_DEVICE_ADDR, REG_I2C_DEVICE_ADDR, self.INPUT_REG)
    if value is None:
      return 0
    return value & 0xFF

  def get_sgx_product_id(self):
    '''!
      @brief Read cached SGX6410 product ID
      @return Sensor product ID, expected 0x2310
      @n Returns 0 when the read fails or the sensor has not been identified yet.
    '''
    value = self.read_u16(REG_I_SGX6410_PID, REG_I2C_SGX6410_PID, self.INPUT_REG)
    if value is None:
      return 0
    return value

  def get_tracking_number(self):
    '''!
      @brief Read the six-byte SGX6410 tracking number
      @return Six-byte list on success, None on failure
    '''
    reg = self.get_reg_addr(REG_I_TRACKING_NUM_0, REG_I2C_TRACKING_NUM_0)
    data = self.read_reg(reg, 6, self.INPUT_REG)
    if (data is None) or (len(data) < 6):
      return None
    return data[0:6]

  def update(self):
    '''!
      @brief Read mode flags and measurement registers, then scale to engineering units
      @return True if registers were read (a repeated sample still returns True with is_new = False)
      @n Call this before get_all_gas_data() / get_single_gas_data().
      @n IAQ/ULP: IAQ = raw/10, TVOC = raw/100 mg/m3, ETOH = raw/100 ppm, ECO2 = raw ppm, RELIAQ = raw/10
      @n PBAQ: TVOC = raw/1000 mg/m3, ETOH = raw/1000 ppm; IAQ, ECO2 and RELIAQ are NAN
      @n is_new and valid come from the bridge.
    '''
    self._gas_data.is_new = False
    mode_raw = self.read_u16(REG_I_SENSORS_MODE, REG_I2C_SENSORS_MODE, self.INPUT_REG)
    if mode_raw is None:
      return False
    self._mode = mode_raw & 0xFF
    if (self._mode != self.IAQ) and (self._mode != self.ULP) and (self._mode != self.PBAQ):
      self.scale_measurement(0, 0, 0, 0, 0)
      return False
    meas = self.read_reg(self.get_reg_addr(REG_I_IAQ, REG_I2C_IAQ), 14, self.INPUT_REG)
    if (meas is None) or (len(meas) < 14):
      return False
    iaq_raw     = meas[0]  | (meas[1]  << 8)
    tvoc_raw    = meas[2]  | (meas[3]  << 8)
    etoh_raw    = meas[4]  | (meas[5]  << 8)
    eco2_raw    = meas[6]  | (meas[7]  << 8)
    rel_iaq_raw = meas[8]  | (meas[9]  << 8)
    is_new_raw  = meas[10] | (meas[11] << 8)
    valid_raw   = meas[12] | (meas[13] << 8)
    self.scale_measurement(iaq_raw, tvoc_raw, etoh_raw, eco2_raw, rel_iaq_raw)
    self._gas_data.is_new = (is_new_raw != 0)
    self._gas_data.valid = (valid_raw != 0)
    return True

  def get_all_gas_data(self):
    '''!
      @brief Return the cached gas measurement structure
      @return GasData Latest cached measurement
      @n Must call update() first. Check is_new for a new sample and valid for warm-up completion.
    '''
    return self._gas_data

  def get_single_gas_data(self, data_type):
    '''!
      @brief Return one value from the cached measurement
      @param data_type Gas value selected by DATA_IAQ / DATA_TVOC / DATA_ETOH / DATA_ECO2 / DATA_RELIAQ
      @return Cached value, or NAN if unsupported in the current mode
      @n Must call update() first.
    '''
    if data_type == self.DATA_IAQ:
      return self._gas_data.iaq
    if data_type == self.DATA_TVOC:
      return self._gas_data.tvoc
    if data_type == self.DATA_ETOH:
      return self._gas_data.etoh
    if data_type == self.DATA_ECO2:
      return self._gas_data.eco2
    if data_type == self.DATA_RELIAQ:
      return self._gas_data.reliaq
    return float('nan')

  def get_sht40_data(self):
    '''!
      @brief Read on-board SHT40 temperature and humidity from the Gravity bridge cache
      @return SHT40Data Cached temperature (C) and humidity (%RH)
      @n Firmware reads SHT40 only after set_auto_humi_compensation(HUM_COMP_ENABLE).
      @n If compensation is off, or no valid sample has been cached yet, temperature and humidity are NAN.
      @n Temperature = -45 + 175 * raw / 65535. Humidity = -6 + 125 * raw / 65535.
    '''
    if self.get_humidity_comp_enable() != HUM_COMP_ENABLE:
      self._sht40_data.temperature = float('nan')
      self._sht40_data.humidity = float('nan')
      return self._sht40_data
    data = self.read_reg(self.get_reg_addr(REG_I_TEMP_RAW, REG_I2C_TEMP_RAW), 4, self.INPUT_REG)
    if (data is None) or (len(data) < 4):
      self._sht40_data.temperature = float('nan')
      self._sht40_data.humidity = float('nan')
      return self._sht40_data
    temp_raw = data[0] | (data[1] << 8)
    hum_raw  = data[2] | (data[3] << 8)
    if (temp_raw == 0) and (hum_raw == 0):
      self._sht40_data.temperature = float('nan')
      self._sht40_data.humidity = float('nan')
      return self._sht40_data
    self._sht40_data.temperature = -45.0 + 175.0 * float(temp_raw) / 65535.0
    self._sht40_data.humidity = -6.0 + 125.0 * float(hum_raw) / 65535.0
    return self._sht40_data

  def write_reg(self, reg, data):
    raise NotImplementedError

  def read_reg(self, reg, length, reg_type=0):
    raise NotImplementedError

  def get_comm_mode(self):
    raise NotImplementedError

  def get_reg_addr(self, uart_reg, i2c_reg):
    if self.get_comm_mode() == self.COMM_MODE_I2C:
      return i2c_reg
    return uart_reg

  def read_u16(self, uart_reg, i2c_reg, reg_type=0):
    reg = self.get_reg_addr(uart_reg, i2c_reg)
    raw = self.read_reg(reg, 2, reg_type)
    if (raw is None) or (len(raw) < 2):
      return None
    return raw[0] | (raw[1] << 8)

  def write_u16(self, uart_reg, i2c_reg, value):
    reg = self.get_reg_addr(uart_reg, i2c_reg)
    raw = [value & 0xFF, (value >> 8) & 0xFF]
    ret = self.write_reg(reg, raw)
    if ret == RET_CODE_OK:
      return True
    return False

  def scale_measurement(self, iaq_raw, tvoc_raw, etoh_raw, eco2_raw, rel_iaq_raw):
    if (self._mode == self.IAQ) or (self._mode == self.ULP):
      self._gas_data.iaq = float(iaq_raw) / 10.0
      self._gas_data.tvoc = float(tvoc_raw) / 100.0
      self._gas_data.etoh = float(etoh_raw) / 100.0
      self._gas_data.eco2 = float(eco2_raw)
      self._gas_data.reliaq = float(rel_iaq_raw) / 10.0
      return
    if self._mode == self.PBAQ:
      self._gas_data.iaq = float('nan')
      self._gas_data.tvoc = float(tvoc_raw) / 1000.0
      self._gas_data.etoh = float(etoh_raw) / 1000.0
      self._gas_data.eco2 = float('nan')
      self._gas_data.reliaq = float('nan')
      return
    self._gas_data.iaq = float('nan')
    self._gas_data.tvoc = float('nan')
    self._gas_data.etoh = float('nan')
    self._gas_data.eco2 = float('nan')
    self._gas_data.reliaq = float('nan')

class DFRobot_SGX6410_Gravity_I2C(DFRobot_SGX6410_Gravity):
  def __init__(self, bus, addr):
    '''!
      @brief Constructor
      @param bus I2C bus number, typically 1 on Raspberry Pi
      @param addr Slave address, 0x52 or 0x53, default 0x53
      @n ADD_SEL low selects 0x52, ADD_SEL high selects 0x53. The switch is latched at module power-up.
    '''
    self._addr = addr
    DFRobot_SGX6410_Gravity.__init__(self, bus, 0, I2C_MODE)

  def begin(self):
    '''!
      @brief Initialize I2C and then verify the module
      @return True on success, False on failure
    '''
    return DFRobot_SGX6410_Gravity.begin(self)

  def get_comm_mode(self):
    return self.COMM_MODE_I2C

  def write_reg(self, reg, data):
    try:
      if isinstance(data, int):
        data = [data]
      buf = [(reg >> 8) & 0xFF] + list(data)
      self.i2cbus.write_i2c_block_data(self._addr, reg & 0xFF, buf)
      return RET_CODE_OK
    except:
      return RET_CODE_ERROR

  def read_reg(self, reg, length, reg_type=0):
    try:
      self.i2cbus.write_i2c_block_data(self._addr, reg & 0xFF, [(reg >> 8) & 0xFF])
      time.sleep(0.01)
      return self.i2cbus.read_i2c_block_data(self._addr, 0, length)
    except:
      return None

class DFRobot_SGX6410_Gravity_UART(DFRobot_SGX6410_Gravity, DFRobot_RTU):
  def __init__(self, baud, addr):
    '''!
      @brief Constructor
      @param baud Baud rate, default 9600
      @param addr Modbus slave address, 0x52 or 0x53, default 0x53
      @n COM_SEL must be UART. Default line settings are 9600 8N1 on /dev/ttyAMA0.
    '''
    self._baud = baud
    self._addr = addr
    try:
      DFRobot_SGX6410_Gravity.__init__(self, 0, baud, UART_MODE)
      DFRobot_RTU.__init__(self, baud, 8, 'N', 1)
    except:
      print("plese get root!")

  def begin(self):
    '''!
      @brief Initialize UART and then verify the module
      @return True on success, False on failure
      @n Sets the Modbus RTU timeout, then runs the base begin() VID check.
    '''
    self.set_timout_time_s(0.5)
    return DFRobot_SGX6410_Gravity.begin(self)

  def get_comm_mode(self):
    return self.COMM_MODE_UART

  def write_reg(self, reg, data):
    if (data is None) or (len(data) == 0) or ((len(data) % 2) != 0):
      return RET_CODE_ERROR
    i = 0
    while i < (len(data) / 2):
      idx = int(i)
      value = data[idx * 2] | (data[idx * 2 + 1] << 8)
      result = self.write_holding_register(self._addr, reg + idx, value)
      if result != 0:
        return RET_CODE_ERROR
      i = i + 1
    return RET_CODE_OK

  def read_reg(self, reg, length, reg_type=0):
    if (length is None) or (length == 0):
      return None
    data = []
    reg_count = int((length + 1) / 2)
    i = 0
    while i < reg_count:
      if reg_type == self.INPUT_REG:
        value = self.read_input_register(self._addr, reg + i)
      else:
        value = self.read_holding_register(self._addr, reg + i)
      if (i * 2) < length:
        data.append(value & 0xFF)
      if (i * 2 + 1) < length:
        data.append((value >> 8) & 0xFF)
      i = i + 1
    return data
