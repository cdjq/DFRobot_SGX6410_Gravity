# DFRobot_SGX6410_Gravity
- [中文版](./README_CN.md)

DFRobot_SGX6410_Gravity is the Arduino library for the Gravity MEMS Air Quality Sensor (SKU: SEN0771). The module uses an SGX6410 gas sensor to estimate indoor air quality and reports IAQ, TVOC, ethanol equivalent (ETOH), estimated CO2 (eCO2), and relative IAQ (RelIAQ). An on-board SHT40 can supply temperature and humidity for automatic humidity compensation.

The host talks to the Gravity module over I2C or UART Modbus RTU, not to the SGX6410 or SHT40 chips on the bus directly. I2C uses a 16-bit little-endian register map. UART defaults to 9600 8N1. The slave address is 0x52 or 0x53, selected by ADD_SEL at power-up; COM_SEL selects I2C or UART.

Typical workflow: call `begin()` to verify the DFRobot vendor ID (`0x3343`), set an operation mode (IAQ / ULP / PBAQ / Suspend), wait until warm-up completes (`valid` is true), then call `update()` and read the cached gas data. Humidity can be written manually as 0–100 %RH. After automatic compensation is enabled, the module internally polls the SHT40 every 60 s.

## Product Link

```
SKU: SEN0771
```

## Table of Contents

  * [Summary](#summary)
  * [Measurement by mode](#measurement-by-mode)
  * [Installation](#installation)
  * [Methods](#methods)
  * [Compatibility](#compatibility)
  * [History](#history)
  * [Credits](#credits)

## Summary

* Dual interface: I2C (16-bit little-endian registers) or UART Modbus RTU (holding `0x03`/`0x06`, input `0x04`), address `0x52` / `0x53`
* UART default line settings: 9600 8N1. ADD_SEL is latched at power-up and is also the Modbus slave address
* `begin()` verifies DFRobot vendor ID `0x3343` and can then read module PID, firmware version, latched address, SGX6410 product ID (`0x2310`) and 6-byte tracking number
* Operation modes: Suspend; IAQ (~3 s sample, ~5 min warm-up); ULP (~90 s sample, ~15 min warm-up); PBAQ (~5 s sample, ~5 min warm-up). Each mode returns a different set of gas fields, see [Measurement by mode](#measurement-by-mode)
* Check `isNew` for a fresh sample and `valid` for warm-up completion before using the readings
* Manual humidity compensation: write 0–100 %RH (stored as code 0–255)
* After automatic humidity compensation is enabled, the module internally polls the SHT40 every 60 s. `getSHT40Data()` returns that cached temperature (°C) and humidity (%RH)
* One-time thermal clean via `runSensorClean()` / `examples/runSensorClean` (about 60 s). Use the API and the demo only once in the sensor lifetime. When it returns, the sensor is already clean and the mode is Suspend. Do not send any further commands after a successful clean. Keep power stable during the sequence.

## Measurement by mode

`update()` / `getAllGasData()` do not always fill every field. IAQ and ULP report the full set. PBAQ only reports TVOC and ETOH (and uses a different scale). Suspend has no measurement payload, so `update()` returns `false`.

| Mode | Line value | Period / warm-up | IAQ | TVOC | ETOH | eCO2 | RelIAQ |
| ---- | ---------- | ---------------- | --- | ---- | ---- | ---- | ------ |
| IAQ | `0x01` | ~3 s / ~5 min | raw/10 | raw/100 mg/m³ | raw/100 ppm | raw ppm | raw/10 |
| ULP | `0x02` | ~90 s / ~15 min | raw/10 | raw/100 mg/m³ | raw/100 ppm | raw ppm | raw/10 |
| PBAQ | `0x05` | ~5 s / ~5 min | NAN | raw/1000 mg/m³ | raw/1000 ppm | NAN | NAN |
| Suspend | `0x00` | — | — | — | — | — | — |

IAQ / ULP sample (`examples/getData`, after warm-up):

```
Humidity comp enable=0
Mode:     0x1
IAQ:      1.60
TVOC:     0.41 mg/m3
ETOH:     0.21 ppm
ECO2:     401.0 ppm
RELIAQ:   1.00
Data valid: YES
--------------------
```

ULP uses the same fields. Only the mode line changes to `Mode:     0x2`, and a new sample arrives about every 90 s.

PBAQ sample (`examples/setOperationMode`). IAQ / eCO2 / RelIAQ print as `nan`:

```
Mode:     0x5
IAQ:      nan
TVOC:     0.04 mg/m3
ETOH:     0.02 ppm
ECO2:     nan ppm
RELIAQ:   nan
Data valid: YES
--------------------
```

Suspend / no measurement:

```
Failed to read gas data!
```

## Installation

* Search `DFRobot_SGX6410_Gravity` in the Arduino IDE Library Manager and install it.
* Or download this repository, unzip it into the Arduino `libraries` folder, then open an example from `examples`.
* UART / Modbus RTU also requires the `DFRobot_RTU` library.

## Methods

```C++
  /**
   * @fn DFRobot_SGX6410_Gravity_UART
   * @brief Constructor (UNO/ESP8266 uses SoftwareSerial)
   * @param sSerial SoftwareSerial object pointer
   * @param baud Baud rate, default 9600
   * @param addr Modbus slave address, 0x52 or 0x53, default 0x53
   * @n ADD_SEL low selects 0x52, ADD_SEL high selects 0x53. The switch is latched at module power-up.
   * @n COM_SEL must be UART. Default line settings are 9600 8N1.
   */
  DFRobot_SGX6410_Gravity_UART(SoftwareSerial *sSerial, uint32_t baud = SGX_UART_BAUD_DEFAULT, uint8_t addr = SGX_ADDR_DEFAULT);

  /**
   * @fn DFRobot_SGX6410_Gravity_UART
   * @brief Constructor (HardwareSerial)
   * @param hSerial HardwareSerial object pointer, typically &Serial1
   * @param baud Baud rate, default 9600
   * @param addr Modbus slave address, 0x52 or 0x53, default 0x53
   * @param rxPin RX pin, 0 means the board default RX
   * @param txPin TX pin, 0 means the board default TX
   * @n ADD_SEL low selects 0x52, ADD_SEL high selects 0x53. The switch is latched at module power-up.
   * @n COM_SEL must be UART. Default line settings are 9600 8N1.
   * @n ESP32: pass rxPin/txPin to remap Serial1, or omit them to use the board default pins.
   */
  DFRobot_SGX6410_Gravity_UART(HardwareSerial *hSerial, uint32_t baud = SGX_UART_BAUD_DEFAULT, uint8_t addr = SGX_ADDR_DEFAULT, uint8_t rxPin = 0, uint8_t txPin = 0);

  /**
   * @fn begin
   * @brief Initialize UART and then verify the module
   * @return bool
   * @retval true  Initialization successful
   * @retval false Initialization failed
   * @n Opens the injected serial port at the constructor baud rate, sets the Modbus RTU timeout, then runs the base begin() VID check.
   * @n On ESP32, custom RX/TX from the constructor are applied here.
   */
  bool begin(void);

  /**
   * @fn DFRobot_SGX6410_Gravity_I2C
   * @brief Constructor
   * @param pWire I2C object pointer, typically &Wire
   * @param addr 7-bit I2C address, 0x52 or 0x53, default 0x53
   * @param sclPin SCL pin, default SGX_I2C_PIN_DEFAULT
   * @param sdaPin SDA pin, default SGX_I2C_PIN_DEFAULT
   * @n ADD_SEL low selects 0x52, ADD_SEL high selects 0x53. The switch is latched at module power-up.
   * @n ESP32/ESP8266: pass sclPin/sdaPin to remap I2C, or omit them to use the board default pins.
   * @n UNO and other boards with fixed Wire pins: omit sclPin/sdaPin; they are ignored in begin().
   */
  DFRobot_SGX6410_Gravity_I2C(TwoWire *pWire, uint8_t addr = SGX_ADDR_DEFAULT, uint8_t sclPin = SGX_I2C_PIN_DEFAULT, uint8_t sdaPin = SGX_I2C_PIN_DEFAULT);

  /**
   * @fn begin
   * @brief Initialize I2C and then verify the module
   * @return bool
   * @retval true  Initialization successful
   * @retval false Initialization failed
   * @n Calls TwoWire::begin() on the injected bus, then the base begin() VID check.
   * @n On ESP32/ESP8266, custom SCL/SDA from the constructor are applied here. UNO always uses the default Wire pins.
   */
  bool begin(void);

  /**
   * @fn begin
   * @brief Initialize the module and verify the vendor ID
   * @return bool
   * @retval true  Initialization successful
   * @retval false Initialization failed
   * @n Reads DEVICE_VID (0x3343). Module PID may still be 0x0000 until the product ID is assigned.
   */
  bool begin(void);

  /**
   * @fn reset
   * @brief Send a sensor reset command to the module
   * @return bool
   * @retval true  Write successful
   * @retval false Write failed
   * @n Host writes 0x0001 to the reset register. The register reads back as 0.
   * @n After reset, wait for the module to finish SGX6410 start-up before calling update().
   */
  bool reset(void);

  /**
   * @fn setOperationMode
   * @brief Set SGX6410 operation mode
   * @param mode Operation mode (see eMode_t)
   * @n eSuspend: Suspend mode
   * @n eIaq: IAQ 2nd generation, about 3 s sample period, 5 min warm-up
   * @n eUlp: Ultra-low power, about 90 s sample period, 15 min warm-up
   * @n ePbaq: PBAQ, about 5 s sample period, 5 min warm-up
   * @n Mode change is a maintenance action. The module restarts warm-up and clears isNew/valid.
   * @n Sensor cleaning 0x80 should be started with runSensorClean(), not called repeatedly.
   * @return bool
   * @retval true  Setting successful
   * @retval false Setting failed or mode not allowed
   */
  bool setOperationMode(eMode_t mode);

  /**
   * @fn runSensorClean
   * @brief Run the sensor thermal-clean sequence: start clean, then poll until done
   * @param timeoutMs Timeout in ms, recommend >= 90000 (clean takes about 60 s)
   * @return uint8_t CLEAN_OK=0 finished / CLEAN_DONE=1 already cleaned / CLEAN_TIMEOUT=2 timeout / CLEAN_FAIL=3 comm error
   * @warning Interrupting clean can permanently damage the sensing material. Keep power stable.
   * @n Use this API and the runSensorClean demo only once in the sensor lifetime.
   * @n When the call returns successfully, the sensor is already clean and the operation mode is Suspend.
   * @n Do not send any further commands after a successful clean.
   * @n Reads REG_I2C_CLEAN_FLAG first. If it is 1, returns CLEAN_DONE without sending 0x80.
   * @n Otherwise writes eSensorClean (0x80) and polls the mode register: 0x80 still cleaning, 0x00 finished (Suspend).
   */
  uint8_t runSensorClean(uint32_t timeoutMs);

  /**
   * @fn getOperationMode
   * @brief Read the current SGX6410 operation mode
   * @return eMode_t Current mode register value
   * @n Returns eSuspend when the read fails.
   */
  eMode_t getOperationMode(void);

  /**
   * @fn setHumidityCompensate
   * @brief Write ambient relative humidity for algorithm compensation
   * @param rh Relative humidity, range 0.0 to 100.0, unit %RH
   * @n Host converts rh to code = round(rh * 255 / 100) and writes the 0-255 code.
   * @n This is a manual write. It does not enable the on-board SHT40 path.
   * @n If automatic compensation is on, the module may overwrite this value on the next SHT40 cycle.
   * @return bool
   * @retval true  Write successful
   * @retval false rh out of range or write failed
   */
  bool setHumidityCompensate(float rh);

  /**
   * @fn getHumidityCompensate
   * @brief Read the last humidity code as %RH
   * @return float Relative humidity in %RH reconstructed from the 0-255 code
   * @n Returns NAN when the register read fails.
   * @n This is the last value written to SGX6410, not a live SHT40 sample.
   */
  float getHumidityCompensate(void);

  /**
   * @fn setAutoHumiCompensation
   * @brief Enable or disable automatic SHT40 humidity compensation on the module
   * @param enable Compensation switch (see eHumComp_t)
   * @n eHumCompDisable: stop SHT40 reads; SGX6410 keeps the last humidity code
   * @n eHumCompEnable: read SHT40 immediately, then the module internally polls SHT40 every 60 s
   * @n Turning off compensation does not restore the SGX6410 default 50 %RH unless you write it with setHumidityCompensate().
   * @return bool
   * @retval true  Write successful
   * @retval false Invalid value or write failed
   */
  bool setAutoHumiCompensation(eHumComp_t enable);

  /**
   * @fn getHumidityCompEnable
   * @brief Read the automatic humidity compensation switch
   * @return eHumComp_t Current switch
   * @n Returns eHumCompDisable when the read fails.
   */
  eHumComp_t getHumidityCompEnable(void);

  /**
   * @fn getVid
   * @brief Read DFRobot vendor ID
   * @return uint16_t Vendor ID, expected 0x3343
   * @n Returns 0 when the read fails.
   */
  uint16_t getVid(void);

  /**
   * @fn getPid
   * @brief Read module product ID
   * @return uint16_t Module PID
   * @n Firmware currently reports 0x0000 until the product ID is assigned.
   * @n Returns 0 when the read fails.
   */
  uint16_t getPid(void);

  /**
   * @fn getFwVersion
   * @brief Read module firmware version
   * @return uint16_t Encoded version, V1.0.0.0 is 0x1000
   * @n Returns 0 when the read fails.
   */
  uint16_t getFwVersion(void);

  /**
   * @fn getDeviceAddr
   * @brief Read the latched external slave address
   * @return uint8_t 7-bit address, 0x52 or 0x53
   * @n DIP ADD_SEL is sampled at power-up and is also the Modbus slave address in UART mode.
   * @n Changing the switch at run time does not take effect until reset.
   * @n Returns 0 when the read fails.
   */
  uint8_t getDeviceAddr(void);

  /**
   * @fn getSgxProductId
   * @brief Read cached SGX6410 product ID
   * @return uint16_t Sensor product ID, expected 0x2310
   * @n Returns 0 when the read fails or the sensor has not been identified yet.
   */
  uint16_t getSgxProductId(void);

  /**
   * @fn getTrackingNumber
   * @brief Read the six-byte SGX6410 tracking number
   * @param number Caller-provided six-byte output buffer
   * @return bool
   * @retval true  Read successful
   * @retval false Invalid pointer or read failed
   */
  bool getTrackingNumber(uint8_t *number);

  /**
   * @fn update
   * @brief Read mode flags and measurement registers, then scale to engineering units
   * @return bool
   * @retval true  Registers were read (a repeated sample still returns true with isNew = false)
   * @retval false Bus error or current mode has no measurement payload
   * @n Call this before getAllGasData() / getSingleGasData().
   * @n IAQ/ULP: IAQ = raw/10, TVOC = raw/100 mg/m3, ETOH = raw/100 ppm, ECO2 = raw ppm, RELIAQ = raw/10
   * @n PBAQ: TVOC = raw/1000 mg/m3, ETOH = raw/1000 ppm; IAQ, ECO2 and RELIAQ are NAN
   * @n isNew and valid come from the module. After a new sample is read, the host writes isNew back to 0.
   */
  bool update(void);

  /**
   * @fn getAllGasData
   * @brief Return the cached gas measurement structure
   * @return const sGasData_t & Latest cached measurement
   * @n Must call update() first. Check isNew for a new sample and valid for warm-up completion.
   */
  const sGasData_t &getAllGasData(void) const;

  /**
   * @fn getSingleGasData
   * @brief Return one value from the cached measurement
   * @param dataType Gas value selected by eDataType_t
   * @n eDataIaq: IAQ
   * @n eDataTvoc: TVOC
   * @n eDataEtoh: ETOH
   * @n eDataEco2: estimated CO2
   * @n eDataRelIaq: relative IAQ
   * @return float Cached value, or NAN if unsupported in the current mode
   * @n Must call update() first.
   */
  float getSingleGasData(eDataType_t dataType);

  /**
   * @fn getSHT40Data
   * @brief Read on-board SHT40 temperature and humidity from the module cache
   * @return sSHT40Data_t & Cached temperature (C) and humidity (%RH)
   * @n Gravity-only API. The host does not talk to SHT40 directly.
   * @n Firmware reads SHT40 only after setAutoHumiCompensation(eHumCompEnable). The module then polls SHT40 internally every 60 s.
   * @n If compensation is off, or no valid sample has been cached yet, temperature and humidity are NAN.
   * @n Temperature = -45 + 175 * raw / 65535. Humidity = -6 + 125 * raw / 65535.
   */
  sSHT40Data_t &getSHT40Data(void);
```

## Compatibility

| Board        | Work Well | Work Wrong | Untested | Remarks |
| ------------ | :-------: | :--------: | :------: | ------- |
| Arduino uno  |           |            |    √     |         |
| Mega2560     |           |            |    √     |         |
| Leonardo     |           |            |    √     |         |
| ESP32        |           |            |    √     |         |
| ESP8266      |           |            |    √     |         |
| micro:bit    |           |            |    √     |         |

## History

- 2026/09/09 - V1.0.0 version

## Credits

Written by PLELES(li.jia@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
