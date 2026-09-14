# DFRobot_SGX6410_Gravity
- [中文版](./README_CN.md)

DFRobot_SGX6410_Gravity is the Raspberry Pi Python library for the Gravity MEMS Air Quality Sensor (SKU: SEN0771). The module uses an SGX6410 gas sensor to estimate indoor air quality and reports IAQ, TVOC, ethanol equivalent (ETOH), estimated CO2 (eCO2), and relative IAQ (RelIAQ). An on-board SHT40 can supply temperature and humidity for automatic humidity compensation.

The host talks to the Gravity module over I2C or UART Modbus RTU, not to the SGX6410 or SHT40 chips on the bus directly. I2C uses a 16-bit little-endian register map on the Pi I2C bus (typically bus 1). UART defaults to 9600 8N1 on `/dev/ttyAMA0`. The slave address is 0x52 or 0x53, selected by ADD_SEL at power-up; COM_SEL selects I2C or UART.

Typical workflow: call `begin()` to verify the DFRobot vendor ID (`0x3343`), set an operation mode (IAQ / ULP / PBAQ / Suspend), wait until warm-up completes (`valid` is true), then call `update()` and read the cached gas data. Humidity can be written manually as 0–100 %RH, or the module can refresh it from the SHT40 about every 60 s after automatic compensation is enabled. In the examples, `ctype = 0` selects I2C and any other value selects UART.

## Product Link

```
SKU: SEN0771
```

## Table of Contents

  * [Summary](#summary)
  * [Installation](#installation)
  * [Methods](#methods)
  * [Compatibility](#compatibility)
  * [History](#history)
  * [Credits](#credits)

## Summary

* Dual interface: I2C (`SMBus`) or UART Modbus RTU (`/dev/ttyAMA0`, holding `0x03`/`0x06`, input `0x04`), address `0x52` / `0x53`
* UART default line settings: 9600 8N1. ADD_SEL is latched at power-up and is also the Modbus slave address
* `begin()` verifies DFRobot vendor ID `0x3343` and can then read module PID, firmware version, latched address, SGX6410 product ID (`0x2310`) and 6-byte tracking number
* Operation modes: Suspend; IAQ (~3 s sample, ~5 min warm-up); ULP (~90 s sample, ~15 min warm-up); PBAQ (~5 s sample, ~5 min warm-up)
* `update()` scales raw registers: IAQ/ULP use IAQ=raw/10, TVOC=raw/100 mg/m³, ETOH=raw/100 ppm, eCO2=raw ppm, RelIAQ=raw/10; PBAQ only reports TVOC/ETOH (raw/1000), other fields are NAN
* Check `is_new` for a fresh sample and `valid` for warm-up completion before using the readings
* Manual humidity compensation: write 0–100 %RH (stored as code 0–255). Automatic SHT40 compensation can refresh that value about every 60 s
* After automatic compensation is on, `get_sht40_data()` returns on-board temperature (°C) and humidity (%RH)
* One-time thermal clean via `run_sensor_clean()` (about 60 s). Do not interrupt power during clean

## Installation

1. Enable I2C and/or UART on the Raspberry Pi (`raspi-config`).
2. Install dependencies:

```
sudo apt-get update
sudo apt-get install python3-smbus python3-serial
```

3. Copy this `python/raspberrypi/` folder onto the Pi, then run an example:

```
python3 examples/get_data/get_data.py
```

Set `ctype = 0` for I2C or a non-zero value for UART. UART also needs `DFRobot_RTU.py` in the same folder as `DFRobot_SGX6410_Gravity.py`.

## Methods

```python
  def __init__(self, bus, addr)
    '''!
      @brief Constructor
      @param bus I2C bus number, typically 1 on Raspberry Pi
      @param addr Slave address, 0x52 or 0x53, default 0x53
      @n ADD_SEL low selects 0x52, ADD_SEL high selects 0x53. The switch is latched at module power-up.
    '''

  def __init__(self, baud, addr)
    '''!
      @brief Constructor
      @param baud Baud rate, default 9600
      @param addr Modbus slave address, 0x52 or 0x53, default 0x53
      @n COM_SEL must be UART. Default line settings are 9600 8N1 on /dev/ttyAMA0.
    '''

  def begin(self)
    '''!
      @brief Initialize the module and verify the vendor ID
      @return True on success, False on failure
      @n Reads DEVICE_VID (0x3343). Module PID may still be 0x0000 until the product ID is assigned.
    '''

  def reset(self)
    '''!
      @brief Send a sensor reset command to the module
      @return True on success, False on failure
      @n Host writes 0x0001 to the reset register. The register reads back as 0.
      @n After reset, wait for the module to finish SGX6410 start-up before calling update().
    '''

  def set_operation_mode(self, mode)
    '''!
      @brief Set SGX6410 operation mode
      @param mode Operation mode
      @n SUSPEND: Suspend mode
      @n IAQ: IAQ 2nd generation, about 3 s sample period, 5 min warm-up
      @n ULP: Ultra-low power, about 90 s sample period, 15 min warm-up
      @n PBAQ: PBAQ, about 5 s sample period, 5 min warm-up
      @n Mode change is a maintenance action. The module restarts warm-up and clears is_new/valid.
      @n Sensor cleaning 0x80 should be started with run_sensor_clean(), not called repeatedly.
      @return True on success, False on failure
    '''

  def run_sensor_clean(self, timeout_ms)
    '''!
      @brief Run the sensor thermal-clean sequence: start clean, then poll until done
      @param timeout_ms Timeout in ms, recommend >= 90000 (clean takes about 60 s)
      @return CLEAN_OK=0 finished / CLEAN_DONE=1 already cleaned / CLEAN_TIMEOUT=2 timeout / CLEAN_FAIL=3 comm error
      @n The sensor should be cleaned only once in its lifetime.
    '''

  def get_operation_mode(self)
    '''!
      @brief Read the current SGX6410 operation mode
      @return Current mode register value
      @n Returns SUSPEND when the read fails.
    '''

  def set_humidity_compensate(self, rh)
    '''!
      @brief Write ambient relative humidity for algorithm compensation
      @param rh Relative humidity, range 0.0 to 100.0, unit %RH
      @n Host converts rh to code = round(rh * 255 / 100) and writes the 0-255 code.
      @n This is a manual write. It does not enable the on-board SHT40 path.
      @return True on success, False on failure
    '''

  def get_humidity_compensate(self)
    '''!
      @brief Read the last humidity code as %RH
      @return Relative humidity in %RH reconstructed from the 0-255 code
      @n Returns NAN when the register read fails.
    '''

  def set_auto_humi_compensation(self, enable)
    '''!
      @brief Enable or disable automatic SHT40 humidity compensation on the module
      @param enable HUM_COMP_DISABLE or HUM_COMP_ENABLE
      @n HUM_COMP_ENABLE: read SHT40 immediately, then about every 60 s
      @return True on success, False on failure
    '''

  def get_humidity_comp_enable(self)
    '''!
      @brief Read the automatic humidity compensation switch
      @return Current switch
      @n Returns HUM_COMP_DISABLE when the read fails.
    '''

  def get_vid(self)
    '''!
      @brief Read DFRobot vendor ID
      @return Vendor ID, expected 0x3343
    '''

  def get_pid(self)
    '''!
      @brief Read module product ID
      @return Module PID
    '''

  def get_fw_version(self)
    '''!
      @brief Read module firmware version
      @return Encoded version, V1.0.0.0 is 0x1000
    '''

  def get_device_addr(self)
    '''!
      @brief Read the latched external slave address
      @return 7-bit address, 0x52 or 0x53
    '''

  def get_sgx_product_id(self)
    '''!
      @brief Read cached SGX6410 product ID
      @return Sensor product ID, expected 0x2310
    '''

  def get_tracking_number(self)
    '''!
      @brief Read the six-byte SGX6410 tracking number
      @return Six-byte list on success, None on failure
    '''

  def update(self)
    '''!
      @brief Read mode flags and measurement registers, then scale to engineering units
      @return True if registers were read
      @n Call this before get_all_gas_data() / get_single_gas_data().
    '''

  def get_all_gas_data(self)
    '''!
      @brief Return the cached gas measurement structure
      @return GasData Latest cached measurement
      @n Must call update() first. Check is_new for a new sample and valid for warm-up completion.
    '''

  def get_single_gas_data(self, data_type)
    '''!
      @brief Return one value from the cached measurement
      @param data_type DATA_IAQ / DATA_TVOC / DATA_ETOH / DATA_ECO2 / DATA_RELIAQ
      @return Cached value, or NAN if unsupported in the current mode
    '''

  def get_sht40_data(self)
    '''!
      @brief Read on-board SHT40 temperature and humidity from the module cache
      @return SHT40Data Cached temperature (C) and humidity (%RH)
      @n Firmware reads SHT40 only after set_auto_humi_compensation(HUM_COMP_ENABLE).
    '''
```

## Compatibility

| Board        | Work Well | Work Wrong | Untested | Remarks |
| ------------ | :-------: | :--------: | :------: | ------- |
| RaspberryPi2 |           |            |    √     |         |
| RaspberryPi3 |           |            |    √     |         |
| RaspberryPi4 |           |            |    √     |         |

* Python Version

| Python  | Work Well | Work Wrong | Untested | Remarks |
| ------- | :-------: | :--------: | :------: | ------- |
| Python2 |           |            |    √     |         |
| Python3 |           |            |    √     |         |

## History

- 2026/09/12 - V1.0.0 version

## Credits

Written by PLELES(li.jia@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
