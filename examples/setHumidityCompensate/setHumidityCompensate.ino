/*!
 * @file setHumidityCompensate.ino
 * @brief Write a manual humidity compensation value
 * @details Disable automatic SHT40 compensation, write 50 %RH, then read the stored value back.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author PLELES(li.jia@dfrobot.com)
 * @version V1.0.0
 * @date 2026-09-09
 * @url https://github.com/DFRobot/DFRobot_SGX6410_Gravity
 */

#include "DFRobot_SGX6410_Gravity.h"

/**
 * I2C address: ADD_SEL low = 0x52, ADD_SEL high = 0x53 (default)
 */
const uint8_t I2C_ADDR = SGX_I2C_ADDR_DEFAULT;
DFRobot_SGX6410_Gravity_I2C sensor(&Wire, I2C_ADDR);

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("Gravity SGX6410 setHumidityCompensate");

  while (sensor.begin() != true) {
    Serial.println("Init failed, check I2C address and wiring");
    delay(1000);
  }

  while (sensor.setHumidityCompEnable(DFRobot_SGX6410_Gravity::eHumCompDisable) != true) {
    Serial.println("Disable auto compensation failed");
    delay(1000);
  }

  while (sensor.setHumidityCompensate(50.0f) != true) {
    Serial.println("Write 50 %RH failed");
    delay(1000);
  }

  Serial.print("Stored humidity %RH=");
  Serial.println(sensor.getHumidityCompensate());
}

void loop()
{
  delay(1000);
}
