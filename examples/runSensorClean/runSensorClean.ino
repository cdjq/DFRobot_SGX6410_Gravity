/*!
 * @file runSensorClean.ino
 * @brief Run the Gravity SGX6410 thermal clean once
 * @details Reads the clean flag first. If the sensor has already been cleaned, skip.
 * @n Otherwise start mode 0x80 and wait until the flag becomes 1, or until timeout.
 * @warning Keep power stable during clean. Do not reset or unplug. Clean only once in the sensor lifetime.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author PLELES(li.jia@dfrobot.com)
 * @version V1.0.0
 * @date 2026-09-11
 * @url https://github.com/DFRobot/DFRobot_SGX6410_Gravity
 */

#include "DFRobot_SGX6410_Gravity.h"

/**
 * I2C address: ADD_SEL low = 0x52, ADD_SEL high = 0x53 (default)
 * ESP32/ESP8266 can also pass SCL/SDA. UNO should omit pin arguments.
 */
const uint8_t I2C_ADDR = SGX_I2C_ADDR_DEFAULT;
DFRobot_SGX6410_Gravity_I2C sgx6410(&Wire, I2C_ADDR);

void setup()
{
  uint8_t result;

  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("Gravity SGX6410 runSensorClean");
  Serial.println("Keep power stable. Clean takes about 60 s. Run only once.");

  while (sgx6410.begin() != true) {
    Serial.println("Init failed, check I2C address and wiring");
    delay(1000);
  }

  result = sgx6410.runSensorClean(90000);
  if (result == CLEAN_OK) {
    Serial.println("Clean finished");
  } else if (result == CLEAN_DONE) {
    Serial.println("Already cleaned, skip");
  } else if (result == CLEAN_TIMEOUT) {
    Serial.println("Clean timeout, do not power off if the sensor is still cleaning");
  } else {
    Serial.println("Clean failed, check I2C");
  }
}

void loop()
{
  delay(1000);
}
