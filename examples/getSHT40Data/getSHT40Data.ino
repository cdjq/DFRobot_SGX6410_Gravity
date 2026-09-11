/*!
 * @file getSHT40Data.ino
 * @brief Read on-board SHT40 temperature and humidity
 * @details Enable automatic humidity compensation, then poll the Gravity bridge SHT40 cache.
 * @n Firmware reads SHT40 only after compensation is enabled.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author PLELES(li.jia@dfrobot.com)
 * @version V1.0.0
 * @date 2026-09-10
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
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("Gravity SGX6410 getSHT40Data");

  while (sgx6410.begin() != true) {
    Serial.println("Init failed, check I2C address and wiring");
    delay(1000);
  }
  Serial.println("Init success");

  while (sgx6410.setHumidityCompEnable(DFRobot_SGX6410_Gravity::eHumCompEnable) != true) {
    Serial.println("Enable humidity compensation failed");
    delay(1000);
  }
  Serial.println("Humidity compensation enabled");
}

void loop()
{
  DFRobot_SGX6410_Gravity::sSHT40Data_t &sht40 = sgx6410.getSHT40Data();
  if (isnan(sht40.temperature) || isnan(sht40.humidity)) {
    Serial.println("SHT40 data not ready, wait for humidity compensation");
  } else {
    Serial.print("Temp:     ");
    Serial.print(sht40.temperature, 2);
    Serial.println(" C");
    Serial.print("Humidity: ");
    Serial.print(sht40.humidity, 2);
    Serial.println(" %RH");
    Serial.println("--------------------");
  }
  delay(1000);
}
