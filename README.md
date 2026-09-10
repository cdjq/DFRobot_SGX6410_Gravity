# DFRobot_SGX6410_Gravity
- [中文版](./README_CN.md)

DFRobot_SGX6410_Gravity is the Arduino library for the Gravity MEMS Air Quality Sensor based on SGX6410. The host talks to a CS32L010 bridge, not to the SGX6410 or SHT40 chips directly. This release implements the external I2C 16-bit little-endian register map used by the module firmware.

The module reports IAQ, TVOC, ethanol equivalent, estimated CO2 and relative IAQ, and can apply humidity compensation from a manual %RH value or from the on-board SHT40.

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

* Supports Gravity SGX6410 module over I2C (7-bit address 0x52 or 0x53)
* Verifies DFRobot vendor ID 0x3343 at begin()
* Configures SGX6410 modes: Suspend, IAQ, ULP, PBAQ
* Reads scaled IAQ, TVOC, ETOH, eCO2 and RelIAQ
* Writes manual humidity compensation codes (0-255 from 0-100 %RH)
* Enables or disables automatic SHT40 humidity compensation on the bridge
* Reads module VID/PID/firmware version, latched I2C address, SGX6410 product ID and tracking number

## Installation

* Search `DFRobot_SGX6410_Gravity` in the Arduino IDE Library Manager and install it.
* Or download this repository, unzip it into the Arduino `libraries` folder, then open an example from `examples`.

## Methods

```C++
  /**
   * @fn DFRobot_SGX6410_Gravity_I2C
   * @brief Constructor
   * @param pWire I2C object pointer, typically &Wire
   * @param addr 7-bit I2C address, 0x52 or 0x53, default 0x53
   * @n ADD_SEL low selects 0x52, ADD_SEL high selects 0x53. The switch is latched at module power-up.
   */
  DFRobot_SGX6410_Gravity_I2C(TwoWire *pWire, uint8_t addr = SGX_I2C_ADDR_DEFAULT);

  /**
   * @fn begin
   * @brief Initialize I2C and then verify the module
   * @return bool
   * @retval true  Initialization successful
   * @retval false Initialization failed
   * @n Calls TwoWire::begin() on the injected bus, then the base begin() VID check.
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
   * @brief Restore sensor reset command on the bridge
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
   * @n Mode change is a maintenance action. The bridge restarts warm-up and clears isNew/valid.
   * @n Sensor cleaning 0x80 is not accepted on this Gravity interface.
   * @return bool
   * @retval true  Setting successful
   * @retval false Setting failed or mode not allowed
   */
  bool setOperationMode(eMode_t mode);

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
   * @n If automatic compensation is on, the bridge may overwrite this value on the next SHT40 cycle.
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
   * @fn setHumidityCompEnable
   * @brief Enable or disable automatic SHT40 humidity compensation on the bridge
   * @param enable Compensation switch (see eHumComp_t)
   * @n eHumCompDisable: stop SHT40 reads; SGX6410 keeps the last humidity code
   * @n eHumCompEnable: read SHT40 immediately, then about every 60 s
   * @n Turning off compensation does not restore the SGX6410 default 50 %RH unless you write it with setHumidityCompensate().
   * @return bool
   * @retval true  Write successful
   * @retval false Invalid value or write failed
   */
  bool setHumidityCompEnable(eHumComp_t enable);

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
   * @n DIP ADD_SEL is sampled at power-up. Changing the switch at run time does not take effect until reset.
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
   * @n isNew and valid come from the bridge, which already applies sample-counter and warm-up rules.
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
