/*!
 * @file DFRobot_SGX6410_Gravity.cpp
 * @brief Implement the DFRobot Gravity SGX6410 air quality module API
 * @details Implements I2C 16-bit little-endian register access and measurement scaling.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author PLELES(li.jia@dfrobot.com)
 * @version V1.0.0
 * @date 2026-09-09
 * @url https://github.com/DFRobot/DFRobot_SGX6410_Gravity
 */

#include "DFRobot_SGX6410_Gravity.h"

DFRobot_SGX6410_Gravity::DFRobot_SGX6410_Gravity(void)
{
  _mode          = eSuspend;
  _gasData.IAQ    = NAN;
  _gasData.TVOC   = NAN;
  _gasData.ETOH   = NAN;
  _gasData.ECO2   = NAN;
  _gasData.RELIAQ = NAN;
  _gasData.isNew         = false;
  _gasData.valid         = false;
  _sht40Data.temperature = NAN;
  _sht40Data.humidity     = NAN;
}

DFRobot_SGX6410_Gravity::~DFRobot_SGX6410_Gravity() {}

uint16_t DFRobot_SGX6410_Gravity::getRegAddr(uint16_t uartReg, uint16_t i2cReg)
{
  if (getCommMode() == eCommModeI2C) {
    return i2cReg;
  } else {
    return uartReg;
  }
}

bool DFRobot_SGX6410_Gravity::readU16(uint16_t uartReg, uint16_t i2cReg, uint16_t &value, eRegType_t regType)
{
  uint16_t reg = getRegAddr(uartReg, i2cReg);
  uint16_t raw = 0;
  uint8_t  ret = readReg(reg, &raw, 2, regType);
  if (ret != RET_CODE_OK) {
    return false;
  }
  value = raw;
  return true;
}

bool DFRobot_SGX6410_Gravity::writeU16(uint16_t uartReg, uint16_t i2cReg, uint16_t value)
{
  uint16_t reg   = getRegAddr(uartReg, i2cReg);
  uint16_t raw   = value;
  uint8_t  ret   = writeReg(reg, &raw, 2);
  return (ret == RET_CODE_OK);
}

void DFRobot_SGX6410_Gravity::scaleMeasurement(uint16_t iaqRaw, uint16_t tvocRaw, uint16_t etohRaw, uint16_t eco2Raw, uint16_t relIaqRaw)
{
  if ((_mode == eIaq) || (_mode == eUlp)) {
    _gasData.IAQ    = (float)iaqRaw / 10.0f;
    _gasData.TVOC   = (float)tvocRaw / 100.0f;
    _gasData.ETOH   = (float)etohRaw / 100.0f;
    _gasData.ECO2   = (float)eco2Raw;
    _gasData.RELIAQ = (float)relIaqRaw / 10.0f;
    return;
  }
  if (_mode == ePbaq) {
    _gasData.IAQ    = NAN;
    _gasData.TVOC   = (float)tvocRaw / 1000.0f;
    _gasData.ETOH   = (float)etohRaw / 1000.0f;
    _gasData.ECO2   = NAN;
    _gasData.RELIAQ = NAN;
    return;
  }
  _gasData.IAQ    = NAN;
  _gasData.TVOC   = NAN;
  _gasData.ETOH   = NAN;
  _gasData.ECO2   = NAN;
  _gasData.RELIAQ = NAN;
}

bool DFRobot_SGX6410_Gravity::begin(void)
{
  uint16_t vid = 0;
  if (!readU16(REG_I_VID, REG_I2C_VID, vid, eInputReg)) {
    DBG("Failed to read vendor ID");
    return false;
  }
  if (vid != DEVICE_VID) {
    DBG("Unexpected vendor ID");
    return false;
  }
  _mode = getOperationMode();
  return true;
}

bool DFRobot_SGX6410_Gravity::reset(void)
{
  if (!writeU16(REG_H_SENSORS_RESET, REG_I2C_SENSORS_RESET, SENSOR_RESET_VALUE)) {
    DBG("Failed to write reset");
    return false;
  }
  delay(50);
  _gasData.isNew  = false;
  _gasData.valid  = false;
  return true;
}

bool DFRobot_SGX6410_Gravity::setOperationMode(eMode_t mode)
{
  if ((mode != eSuspend) && (mode != eIaq) && (mode != eUlp) && (mode != ePbaq) && (mode != eSensorClean)) {
    DBG("Invalid operation mode");
    return false;
  }
  if (!writeU16(REG_H_SENSORS_MODE, REG_I2C_SENSORS_MODE, (uint16_t)mode)) {
    DBG("Failed to set operation mode");
    return false;
  }
  delay(50);
  _mode           = mode;
  _gasData.isNew  = false;
  _gasData.valid  = false;
  return true;
}

uint8_t DFRobot_SGX6410_Gravity::runSensorClean(uint32_t timeoutMs)
{
  uint16_t flag = 0;
  uint16_t mode = 0;
  uint32_t start;

  if (!readU16(REG_H_CLEAN_FLAG, REG_I2C_CLEAN_FLAG, flag, eHoldingReg)) {
    DBG("Failed to read clean flag");
    return CLEAN_FAIL;
  }
  if ((flag & 0xFF) == 1) {
    return CLEAN_DONE;
  }

  if (!setOperationMode(eSensorClean)) {
    DBG("Failed to start sensor clean");
    return CLEAN_FAIL;
  }

  start = millis();
  while ((uint32_t)(millis() - start) < timeoutMs) {
    delay(1000);
    if (!readU16(REG_I_SENSORS_MODE, REG_I2C_SENSORS_MODE, mode, eInputReg)) {
      DBG("Failed to poll operation mode");
      return CLEAN_FAIL;
    }
    mode &= 0xFF;
    if (mode == (uint16_t)eSuspend) {
      _mode = eSuspend;
      return CLEAN_OK;
    }
  }
  return CLEAN_TIMEOUT;
}

DFRobot_SGX6410_Gravity::eMode_t DFRobot_SGX6410_Gravity::getOperationMode(void)
{
  uint16_t value = 0;
  if (!readU16(REG_I_SENSORS_MODE, REG_I2C_SENSORS_MODE, value, eInputReg)) {
    DBG("Failed to read operation mode");
    return eSuspend;
  }
  _mode = (eMode_t)(value & 0xFF);
  return _mode;
}

bool DFRobot_SGX6410_Gravity::setHumidityCompensate(float rh)
{
  if (isnan(rh) || (rh < 0.0f) || (rh > 100.0f)) {
    DBG("Humidity out of range");
    return false;
  }
  uint8_t  code = (uint8_t)((rh * 255.0f / 100.0f) + 0.5f);
  uint16_t raw  = (uint16_t)code;
  if (!writeU16(REG_H_HUMIDITY, REG_I2C_HUMIDITY, raw)) {
    DBG("Failed to write humidity");
    return false;
  }
  delay(50);
  return true;
}

float DFRobot_SGX6410_Gravity::getHumidityCompensate(void)
{
  uint16_t raw = 0;
  if (!readU16(REG_H_HUMIDITY, REG_I2C_HUMIDITY, raw, eHoldingReg)) {
    return NAN;
  }
  return ((float)(raw & 0xFF) * 100.0f) / 255.0f;
}

bool DFRobot_SGX6410_Gravity::setHumidityCompEnable(eHumComp_t enable)
{
  if ((enable != eHumCompDisable) && (enable != eHumCompEnable)) {
    DBG("Invalid humidity compensation switch");
    return false;
  }
  if (!writeU16(REG_H_HUM_COMP_EN, REG_I2C_HUM_COMP_EN, (uint16_t)enable)) {
    DBG("Failed to write humidity compensation switch");
    return false;
  }
  delay(50);
  return true;
}

DFRobot_SGX6410_Gravity::eHumComp_t DFRobot_SGX6410_Gravity::getHumidityCompEnable(void)
{
  uint16_t raw = 0;
  if (!readU16(REG_H_HUM_COMP_EN, REG_I2C_HUM_COMP_EN, raw, eHoldingReg)) {
    return eHumCompDisable;
  }
  if (raw == HUM_COMP_ENABLE) {
    return eHumCompEnable;
  }
  return eHumCompDisable;
}

uint16_t DFRobot_SGX6410_Gravity::getVid(void)
{
  uint16_t value = 0;
  if (!readU16(REG_I_VID, REG_I2C_VID, value, eInputReg)) {
    return 0;
  }
  return value;
}

uint16_t DFRobot_SGX6410_Gravity::getPid(void)
{
  uint16_t value = 0;
  if (!readU16(REG_I_PID, REG_I2C_PID, value, eInputReg)) {
    return 0;
  }
  return value;
}

uint16_t DFRobot_SGX6410_Gravity::getFwVersion(void)
{
  uint16_t value = 0;
  if (!readU16(REG_I_VERSION, REG_I2C_FW_VERSION, value, eInputReg)) {
    return 0;
  }
  return value;
}

uint8_t DFRobot_SGX6410_Gravity::getDeviceAddr(void)
{
  uint16_t value = 0;
  if (!readU16(REG_I_DEVICE_ADDR, REG_I2C_DEVICE_ADDR, value, eInputReg)) {
    return 0;
  }
  return (uint8_t)(value & 0xFF);
}

uint16_t DFRobot_SGX6410_Gravity::getSgxProductId(void)
{
  uint16_t value = 0;
  if (!readU16(REG_I_SGX6410_PID, REG_I2C_SGX6410_PID, value, eInputReg)) {
    return 0;
  }
  return value;
}

bool DFRobot_SGX6410_Gravity::getTrackingNumber(uint8_t *number)
{
  uint8_t data[6] = {0, 0, 0, 0, 0, 0};
  if (number == NULL) {
    return false;
  }
  uint16_t reg = getRegAddr(REG_I_TRACKING_NUM_0, REG_I2C_TRACKING_NUM_0);
  if (readReg(reg, data, 6, eInputReg) != RET_CODE_OK) {
    DBG("Failed to read tracking number");
    return false;
  }
  for (uint8_t i = 0; i < 6; i++) {
    number[i] = data[i];
  }
  return true;
}

bool DFRobot_SGX6410_Gravity::update(void)
{
  uint16_t modeRaw  = 0;
  uint8_t  meas[14] = {0};
  uint16_t iaqRaw;
  uint16_t tvocRaw;
  uint16_t etohRaw;
  uint16_t eco2Raw;
  uint16_t relIaqRaw;
  uint16_t isNewRaw;
  uint16_t validRaw;

  _gasData.isNew = false;
  if (!readU16(REG_I_SENSORS_MODE, REG_I2C_SENSORS_MODE, modeRaw, eInputReg)) {
    DBG("Failed to read mode");
    return false;
  }
  _mode = (eMode_t)(modeRaw & 0xFF);
  if ((_mode != eIaq) && (_mode != eUlp) && (_mode != ePbaq)) {
    scaleMeasurement(0, 0, 0, 0, 0);
    return false;
  }

  if (readReg(getRegAddr(REG_I_IAQ, REG_I2C_IAQ), meas, 14, eInputReg) != RET_CODE_OK) {
    DBG("Failed to read measurement");
    return false;
  }

  iaqRaw    = (uint16_t)meas[0]  | ((uint16_t)meas[1]  << 8);
  tvocRaw   = (uint16_t)meas[2]  | ((uint16_t)meas[3]  << 8);
  etohRaw   = (uint16_t)meas[4]  | ((uint16_t)meas[5]  << 8);
  eco2Raw   = (uint16_t)meas[6]  | ((uint16_t)meas[7]  << 8);
  relIaqRaw = (uint16_t)meas[8]  | ((uint16_t)meas[9]  << 8);
  isNewRaw  = (uint16_t)meas[10] | ((uint16_t)meas[11] << 8);
  validRaw  = (uint16_t)meas[12] | ((uint16_t)meas[13] << 8);

  scaleMeasurement(iaqRaw, tvocRaw, etohRaw, eco2Raw, relIaqRaw);
  _gasData.isNew = (isNewRaw != 0);
  _gasData.valid = (validRaw != 0);
  return true;
}

float DFRobot_SGX6410_Gravity::getSingleGasData(eDataType_t dataType)
{
  switch (dataType) {
    case eDataIaq:
      return _gasData.IAQ;
    case eDataTvoc:
      return _gasData.TVOC;
    case eDataEtoh:
      return _gasData.ETOH;
    case eDataEco2:
      return _gasData.ECO2;
    case eDataRelIaq:
      return _gasData.RELIAQ;
    default:
      return NAN;
  }
}

DFRobot_SGX6410_Gravity::sSHT40Data_t &DFRobot_SGX6410_Gravity::getSHT40Data(void)
{
  uint8_t  data[4] = {0, 0, 0, 0};
  uint16_t tempRaw;
  uint16_t humRaw;

  if (getHumidityCompEnable() != eHumCompEnable) {
    _sht40Data.temperature = NAN;
    _sht40Data.humidity    = NAN;
    return _sht40Data;
  }

  if (readReg(getRegAddr(REG_I_TEMP_RAW, REG_I2C_TEMP_RAW), data, 4, eInputReg) != RET_CODE_OK) {
    DBG("Failed to read SHT40 cache");
    _sht40Data.temperature = NAN;
    _sht40Data.humidity    = NAN;
    return _sht40Data;
  }

  tempRaw = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
  humRaw  = (uint16_t)data[2] | ((uint16_t)data[3] << 8);
  if ((tempRaw == 0) && (humRaw == 0)) {
    _sht40Data.temperature = NAN;
    _sht40Data.humidity    = NAN;
    return _sht40Data;
  }
  _sht40Data.temperature = -45.0f + 175.0f * (float)tempRaw / 65535.0f;
  _sht40Data.humidity    = -6.0f + 125.0f * (float)humRaw / 65535.0f;
  return _sht40Data;
}

DFRobot_SGX6410_Gravity_I2C::DFRobot_SGX6410_Gravity_I2C(TwoWire *pWire, uint8_t addr, uint8_t sclPin, uint8_t sdaPin) : _pWire(pWire), _address(addr), _sclPin(sclPin), _sdaPin(sdaPin) {}

DFRobot_SGX6410_Gravity_I2C::~DFRobot_SGX6410_Gravity_I2C() {}

DFRobot_SGX6410_Gravity::eCommMode_t DFRobot_SGX6410_Gravity_I2C::getCommMode(void)
{
  return DFRobot_SGX6410_Gravity::eCommModeI2C;
}

bool DFRobot_SGX6410_Gravity_I2C::begin(void)
{
#if defined(ESP32) || defined(ESP8266)
  if ((_sclPin != SGX_I2C_PIN_DEFAULT) || (_sdaPin != SGX_I2C_PIN_DEFAULT)) {
    _pWire->begin(_sdaPin, _sclPin);
  } else {
    _pWire->begin();
  }
#else
  _pWire->begin();
#endif
#if defined(ARDUINO_ARCH_ESP32)
  _pWire->setTimeOut(100);
#elif defined(ARDUINO_ARCH_ESP8266)
  _pWire->setClockStretchLimit(150000);
#elif defined(WIRE_HAS_TIMEOUT)
  _pWire->setWireTimeout(25000, true);
#endif
  return DFRobot_SGX6410_Gravity::begin();
}

uint8_t DFRobot_SGX6410_Gravity_I2C::writeReg(uint16_t reg, void *data, uint8_t len)
{
  uint8_t *pData = (uint8_t *)data;

  _pWire->beginTransmission(_address);
  _pWire->write((uint8_t)(reg & 0xFF));
  _pWire->write((uint8_t)((reg >> 8) & 0xFF));
  for (uint8_t i = 0; i < len; i++) {
    _pWire->write(pData[i]);
  }
  if (_pWire->endTransmission() != 0) {
    DBG("I2C write failed");
    return RET_CODE_ERROR;
  }
  return RET_CODE_OK;
}

uint8_t DFRobot_SGX6410_Gravity_I2C::readReg(uint16_t reg, void *data, uint8_t len, eRegType_t regType)
{
  (void)regType;
  uint8_t *pData = (uint8_t *)data;

  _pWire->beginTransmission(_address);
  _pWire->write((uint8_t)(reg & 0xFF));
  _pWire->write((uint8_t)((reg >> 8) & 0xFF));

  if (_pWire->endTransmission() != 0) {
    DBG("I2C write register address failed");
    return RET_CODE_ERROR;
  }

  delay(10);

  _pWire->requestFrom(_address, len);
  if (_pWire->available() < len) {
    DBG("I2C read failed, not enough data");
    return RET_CODE_ERROR;
  }
  for (uint8_t i = 0; i < len; i++) {
    pData[i] = _pWire->read();
  }
  return RET_CODE_OK;
}
