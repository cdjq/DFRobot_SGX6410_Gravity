/*!
 * @file DFRobot_SGX6410_Gravity.h
 * @brief Define basic struct of DFRobot_SGX6410_Gravity class
 * @details Header for DFRobot_SGX6410_Gravity: class declaration, protocol constants, enums and structs.
 * @n Gravity MEMS Air Quality Sensor (SGX6410) talks to the host through a CS32L010 bridge.
 * @n This release implements the external I2C register map and UART Modbus RTU.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author PLELES(li.jia@dfrobot.com)
 * @version V1.0.0
 * @date 2026-09-09
 * @url https://github.com/DFRobot/DFRobot_SGX6410_Gravity
 */
#ifndef __DFROBOT_SGX6410_GRAVITY_H__
#define __DFROBOT_SGX6410_GRAVITY_H__

#include "Arduino.h"
#include "Wire.h"
#include "DFRobot_RTU.h"
#include "stdint.h"
#include <math.h>

#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
#include "SoftwareSerial.h"
#else
#include "HardwareSerial.h"
#endif

//#define ENABLE_DBG

#ifdef ENABLE_DBG
#define DBG(...)                 \
  {                              \
    Serial.print("[");           \
    Serial.print(__FUNCTION__);  \
    Serial.print("(): ");        \
    Serial.print(__LINE__);      \
    Serial.print(" ] ");         \
    Serial.println(__VA_ARGS__); \
  }
#else
#define DBG(...)
#endif

#define SGX_ADDR_LOW              0x52      ///< Slave address when ADD_SEL is low
#define SGX_ADDR_HIGH             0x53      ///< Slave address when ADD_SEL is high (default)
#define SGX_ADDR_DEFAULT          0x53      ///< Default slave address
#define SGX_I2C_PIN_DEFAULT       0xFF      ///< Use the board default SDA/SCL pins
#define SGX_UART_BAUD_DEFAULT     9600      ///< Default UART baud rate (holding code 0x0003)
#define DEVICE_VID                0x3343    ///< DFRobot vendor ID
#define DEVICE_PID                0x0000    ///< Module product ID (firmware TBD)
#define DEVICE_FW_VERSION         0x1000    ///< Module firmware version encoding V1.0.0.0
#define SGX_PRODUCT_ID            0x2310    ///< SGX6410 sensor product ID

#define HUM_COMP_DISABLE         0x0000    ///< Automatic SHT40 humidity compensation off
#define HUM_COMP_ENABLE          0x0001    ///< Automatic SHT40 humidity compensation on
#define SENSOR_RESET_VALUE       0x0001    ///< Write this value to trigger sensors reset

#define SGX_WARMUP_IAQ_MS        300000UL  ///< IAQ warm-up time in ms
#define SGX_WARMUP_ULP_MS        900000UL  ///< ULP warm-up time in ms
#define SGX_WARMUP_PBAQ_MS       300000UL  ///< PBAQ warm-up time in ms
#define SGX_POLL_IAQ_MS          3000UL    ///< IAQ recommended poll period in ms
#define SGX_POLL_ULP_MS          90000UL   ///< ULP recommended poll period in ms
#define SGX_POLL_PBAQ_MS         5000UL    ///< PBAQ recommended poll period in ms

/*****************************UARTModbus registers*********************************************** */
#define REG_I_PID                 0x0000    ///< UART input: module product ID
#define REG_I_VID                 0x0001    ///< UART input: vendor ID
#define REG_I_DEVICE_ADDR         0x0002    ///< UART input: device address
#define REG_I_RESERVED0           0x0003    ///< UART input: reserved
#define REG_I_RESERVED1           0x0004    ///< UART input: reserved
#define REG_I_VERSION             0x0005    ///< UART input: firmware version
#define REG_I_SGX6410_PID         0x0006    ///< UART input: SGX6410 product ID
#define REG_I_TRACKING_NUM_0      0x0007    ///< UART input: tracking number bytes 0-1
#define REG_I_TRACKING_NUM_1      0x0008    ///< UART input: tracking number bytes 2-3
#define REG_I_TRACKING_NUM_2      0x0009    ///< UART input: tracking number bytes 4-5
#define REG_I_IAQ                 0x000A    ///< UART input: IAQ raw value
#define REG_I_TVOC                0x000B    ///< UART input: TVOC raw value
#define REG_I_ETOH                0x000C    ///< UART input: ETOH raw value
#define REG_I_ECO2                0x000D    ///< UART input: eCO2 raw value
#define REG_I_RELIAQ              0x000E    ///< UART input: relative IAQ raw value
#define REG_I_DATA_IS_NEW         0x000F    ///< UART input: new-sample flag
#define REG_I_DATA_VALID          0x0010    ///< UART input: warm-up valid flag
#define REG_I_SENSORS_MODE        0x0011    ///< UART input: SGX6410 operating mode
#define REG_I_TEMP_RAW            0x0012    ///< UART input: SHT40 temperature ticks
#define REG_I_HUM_RAW             0x0013    ///< UART input: SHT40 humidity ticks

#define REG_H_RESERVED0           0x0000    ///< UART holding: reserved
#define REG_H_RESERVED1           0x0001    ///< UART holding: reserved
#define REG_H_RESERVED2           0x0002    ///< UART holding: reserved
#define REG_H_BAUDRATE            0x0003    ///< UART holding: baud rate
#define REG_H_VERIFY_AND_STOP     0x0004    ///< UART holding: parity and stop bits
#define REG_H_RESERVED3           0x0005    ///< UART holding: reserved
#define REG_H_SENSORS_MODE        0x0006    ///< UART holding: SGX6410 operating mode
#define REG_H_SENSORS_RESET       0x0007    ///< UART holding: write 0x0001 to reset
#define REG_H_HUMIDITY            0x0008    ///< UART holding: humidity code 0-255
#define REG_H_HUM_COMP_EN         0x0009    ///< UART holding: humidity compensation enable
#define REG_H_CLEAN_FLAG          0x000A    ///< UART holding: 0=not cleaned, 1=cleaned (once in lifetime)

/*****************************I2C registers*********************************************** */
#define REG_I2C_VID               0x0000    ///< I2C: vendor ID
#define REG_I2C_PID               0x0001    ///< I2C: module product ID
#define REG_I2C_FW_VERSION        0x0002    ///< I2C: module firmware version
#define REG_I2C_DEVICE_ADDR       0x0003    ///< I2C: slave address
#define REG_I2C_SENSORS_RESET     0x0004    ///< I2C: write 0x0001 to reset, reads 0
#define REG_I2C_SENSORS_MODE      0x0005    ///< I2C: SGX6410 operating mode
#define REG_I2C_HUMIDITY          0x0006    ///< I2C: last humidity code written to SGX6410
#define REG_I2C_HUM_COMP_EN       0x0007    ///< I2C: humidity compensation enable
#define REG_I2C_SGX6410_PID       0x0008    ///< I2C: SGX6410 product ID
#define REG_I2C_TRACKING_NUM_0    0x0009    ///< I2C: tracking number bytes 0-1
#define REG_I2C_TRACKING_NUM_1    0x000A    ///< I2C: tracking number bytes 2-3
#define REG_I2C_TRACKING_NUM_2    0x000B    ///< I2C: tracking number bytes 4-5
#define REG_I2C_IAQ               0x000C    ///< I2C: IAQ raw value
#define REG_I2C_TVOC              0x000D    ///< I2C: TVOC raw value
#define REG_I2C_ETOH              0x000E    ///< I2C: ETOH raw value
#define REG_I2C_ECO2              0x000F    ///< I2C: eCO2 raw value
#define REG_I2C_RELIAQ            0x0010    ///< I2C: relative IAQ raw value
#define REG_I2C_DATA_IS_NEW       0x0011    ///< I2C: 1 = new sample, 0 = not updated
#define REG_I2C_DATA_VALID        0x0012    ///< I2C: 1 = warm-up complete, 0 = invalid
#define REG_I2C_TEMP_RAW          0x0013    ///< I2C: SHT40 temperature ticks, T=-45+175*raw/65535
#define REG_I2C_HUM_RAW           0x0014    ///< I2C: SHT40 humidity ticks, RH=-6+125*raw/65535
#define REG_I2C_CLEAN_FLAG        0x0015    ///< I2C: 0=not cleaned, 1=cleaned (2 bytes, g_I2CRegMap[42..43])

class DFRobot_SGX6410_Gravity {
public:
#define RET_CODE_OK    0    ///< Return code: success
#define RET_CODE_ERROR 1    ///< Return code: failure
#define CLEAN_OK       0    ///< Sensor clean finished
#define CLEAN_DONE     1    ///< Sensor has already been cleaned
#define CLEAN_TIMEOUT  2    ///< Clean did not finish before timeout
#define CLEAN_FAIL     3    ///< Communication failed during clean

  /**
   * @enum eCommMode_t
   * @brief Communication mode enumeration
   */
  typedef enum {
    eCommModeUART = 0,    /**< UART communication mode (Modbus RTU) */
    eCommModeI2C  = 1     /**< I2C communication mode */
  } eCommMode_t;

  /**
   * @enum eRegType_t
   * @brief Register type enumeration (for UART Modbus mode)
   * @n In UART mode, input registers (function code 0x04) and holding registers (function code 0x03)
   * @n have overlapping address ranges, so this enum is needed to distinguish them.
   * @n In I2C mode, this parameter is ignored as there is only one register space.
   */
  typedef enum {
    eInputReg   = 0,    /**< Input register (read-only, Modbus function code 0x04) */
    eHoldingReg = 1     /**< Holding register (read-write, Modbus function code 0x03/0x06) */
  } eRegType_t;

  /**
   * @enum eMode_t
   * @brief SGX6410 operation mode
   * @n Line values match the sensor/firmware command bytes.
   */
  typedef enum {
    eSuspend     = 0x00,    /**< Suspend mode */
    eIaq         = 0x01,    /**< IAQ 2nd generation mode, poll about every 3 s, warm-up 5 min */
    eUlp         = 0x02,    /**< Ultra-low power mode, poll about every 90 s, warm-up 15 min */
    ePbaq        = 0x05,    /**< PBAQ mode, poll about every 5 s, warm-up 5 min */
    eSensorClean = 0x80    /**< Thermal clean, once in lifetime */
  } eMode_t;

  /**
   * @enum eDataType_t
   * @brief Select one cached gas value
   */
  typedef enum {
    eDataIaq    = 0,    /**< IAQ, UBA level */
    eDataTvoc   = 1,    /**< TVOC */
    eDataEtoh   = 2,    /**< Ethanol equivalent */
    eDataEco2   = 3,    /**< Estimated CO2 */
    eDataRelIaq = 4     /**< Relative IAQ */
  } eDataType_t;

  /**
   * @enum eHumComp_t
   * @brief Automatic humidity compensation switch
   */
  typedef enum {
    eHumCompDisable = HUM_COMP_DISABLE,    /**< Do not read SHT40 or send humidity to SGX6410 */
    eHumCompEnable  = HUM_COMP_ENABLE     /**< Read SHT40 and write humidity code to SGX6410 */
  } eHumComp_t;

  /**
   * @struct sGasData_t
   * @brief Parsed and cached gas measurement data
   */
  typedef struct {
    float IAQ;     /**< IAQ, UBA level; NAN in PBAQ or Suspend */
    float TVOC;    /**< TVOC, mg/m3 */
    float ETOH;    /**< Ethanol equivalent, ppm */
    float ECO2;    /**< Estimated CO2, ppm; NAN in PBAQ or Suspend */
    float RELIAQ;  /**< Relative IAQ; NAN in PBAQ or Suspend */
    bool  isNew;   /**< True when the bridge reports a new sample counter */
    bool  valid;   /**< True when the sample was acquired after mode warm-up */
  } sGasData_t;

  /**
   * @struct sSHT40Data_t
   * @brief On-board SHT40 temperature and humidity
   */
  typedef struct {
    float temperature;    /**< Temperature, unit Celsius */
    float humidity;        /**< Relative humidity, unit %RH */
  } sSHT40Data_t;

  /**
   * @fn DFRobot_SGX6410_Gravity
   * @brief Constructor
   */
  DFRobot_SGX6410_Gravity(void);
  ~DFRobot_SGX6410_Gravity();

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
   * @n The sensor should be cleaned only once in its lifetime.
   * @n Reads REG_I2C_CLEAN_FLAG first. If it is 1, returns CLEAN_DONE without sending 0x80.
   * @n Otherwise writes eSensorClean (0x80) and polls the mode register: 0x80 still cleaning, 0x00 finished.
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
   * @fn setAutoHumiCompensation
   * @brief Enable or disable automatic SHT40 humidity compensation on the bridge
   * @param enable Compensation switch (see eHumComp_t)
   * @n eHumCompDisable: stop SHT40 reads; SGX6410 keeps the last humidity code
   * @n eHumCompEnable: read SHT40 immediately, then about every 60 s
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
   * @n isNew and valid come from the bridge.
   */
  bool update(void);

  /**
   * @fn getAllGasData
   * @brief Return the cached gas measurement structure
   * @return sGasData_t & Latest cached measurement
   * @n Must call update() first. Check isNew for a new sample and valid for warm-up completion.
   */
  sGasData_t &getAllGasData(void) { return _gasData; }

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
   * @brief Read on-board SHT40 temperature and humidity from the Gravity bridge cache
   * @return sSHT40Data_t & Cached temperature (C) and humidity (%RH)
   * @n Gravity-only API. The host does not talk to SHT40 directly.
   * @n Firmware reads SHT40 only after setAutoHumiCompensation(eHumCompEnable).
   * @n If compensation is off, or no valid sample has been cached yet, temperature and humidity are NAN.
   * @n Temperature = -45 + 175 * raw / 65535. Humidity = -6 + 125 * raw / 65535.
   */
  sSHT40Data_t &getSHT40Data(void);

protected:
  virtual uint8_t writeReg(uint16_t reg, void *data, uint8_t len) = 0;
  virtual uint8_t readReg(uint16_t reg, void *data, uint8_t len, eRegType_t regType = eInputReg) = 0;
  virtual eCommMode_t getCommMode(void) = 0;
  uint16_t getRegAddr(uint16_t uartReg, uint16_t i2cReg);
  bool readU16(uint16_t uartReg, uint16_t i2cReg, uint16_t &value, eRegType_t regType = eInputReg);
  bool writeU16(uint16_t uartReg, uint16_t i2cReg, uint16_t value);
  void scaleMeasurement(uint16_t iaqRaw, uint16_t tvocRaw, uint16_t etohRaw, uint16_t eco2Raw, uint16_t relIaqRaw);

  sGasData_t   _gasData;
  sSHT40Data_t _sht40Data;
  eMode_t      _mode;
};

class DFRobot_SGX6410_Gravity_I2C : public DFRobot_SGX6410_Gravity {
public:
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
   * @fn ~DFRobot_SGX6410_Gravity_I2C
   * @brief Destructor
   */
  ~DFRobot_SGX6410_Gravity_I2C();

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

protected:
  uint8_t writeReg(uint16_t reg, void *data, uint8_t len);
  uint8_t readReg(uint16_t reg, void *data, uint8_t len, eRegType_t regType = eInputReg);
  eCommMode_t getCommMode(void);

private:
  TwoWire * _pWire;
  uint8_t   _address;
  uint8_t   _sclPin;
  uint8_t   _sdaPin;
};

class DFRobot_SGX6410_Gravity_UART : public DFRobot_SGX6410_Gravity, public DFRobot_RTU {
public:
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
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
#else
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
#endif
  /**
   * @fn ~DFRobot_SGX6410_Gravity_UART
   * @brief Destructor
   */
  ~DFRobot_SGX6410_Gravity_UART();

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

protected:
  uint8_t writeReg(uint16_t reg, void *data, uint8_t len);
  uint8_t readReg(uint16_t reg, void *data, uint8_t len, eRegType_t regType = eInputReg);
  eCommMode_t getCommMode(void);

private:
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
  SoftwareSerial * _serial;
#else
  HardwareSerial * _serial;
#endif
  uint32_t _baud;
  uint8_t  _rxPin;
  uint8_t  _txPin;
  uint8_t  _deviceAddr;
};

#endif
