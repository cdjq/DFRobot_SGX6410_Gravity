/*!
 * @file getDataWithHumidityComp.ino
 * @brief Read Gravity SGX6410 air quality data with humidity compensation
 * @details Enable automatic SHT40 humidity compensation, then poll all gas data,
 * @n SHT40 temperature/humidity, and the humidity compensation switch.
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
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Gravity SGX6410 getDataWithHumidityComp");
  Serial.flush();

  while (sgx6410.begin() != true) {
    Serial.println("Init failed, check I2C address and wiring");
    Serial.flush();
    delay(1000);
  }
  Serial.println("Init success");

  Serial.print("VID=0x");
  Serial.println(sgx6410.getVid(), HEX);
  Serial.print("SGX PID=0x");
  Serial.println(sgx6410.getSgxProductId(), HEX);

  uint8_t tracking[6] = {0, 0, 0, 0, 0, 0};
  if (sgx6410.getTrackingNumber(tracking)) {
    Serial.print("Tracking=");
    for (uint8_t i = 0; i < 6; i++) {
      if (tracking[i] < 0x10) {
        Serial.print("0");
      }
      Serial.print(tracking[i], HEX);
      if (i < 5) {
        Serial.print(" ");
      }
    }
    Serial.println();
  }

  while (sgx6410.setOperationMode(DFRobot_SGX6410_Gravity::eIaq) != true) {
    Serial.println("Set IAQ mode failed");
    delay(1000);
  }
  Serial.println("IAQ mode set");

  while (sgx6410.setHumidityCompEnable(DFRobot_SGX6410_Gravity::eHumCompEnable) != true) {
    Serial.println("Enable humidity compensation failed");
    delay(1000);
  }
  Serial.println("Humidity compensation enabled");
}

void loop()
{
  if (sgx6410.update()) {
    const DFRobot_SGX6410_Gravity::sGasData_t &data = sgx6410.getAllGasData();
    if (!data.isNew) {
      delay(500);
      return;
    }
    Serial.print("Humidity comp enable=");
    Serial.println((uint16_t)sgx6410.getHumidityCompEnable());
    Serial.print("Mode:     0x");
    Serial.println((uint8_t)sgx6410.getOperationMode(), HEX);
    Serial.print("IAQ:      ");
    Serial.println(data.IAQ, 2);
    Serial.print("TVOC:     ");
    Serial.print(data.TVOC, 2);
    Serial.println(" mg/m3");
    Serial.print("ETOH:     ");
    Serial.print(data.ETOH, 2);
    Serial.println(" ppm");
    Serial.print("ECO2:     ");
    Serial.print(data.ECO2, 1);
    Serial.println(" ppm");
    Serial.print("RELIAQ:   ");
    Serial.println(data.RELIAQ, 2);
    Serial.print("Data valid: ");
    Serial.println(data.valid ? "YES" : "NO (warming up)");
    DFRobot_SGX6410_Gravity::sSHT40Data_t &sht40 = sgx6410.getSHT40Data();
    Serial.print("Temp:     ");
    Serial.print(sht40.temperature, 2);
    Serial.println(" C");
    Serial.print("Humidity: ");
    Serial.print(sht40.humidity, 2);
    Serial.println(" %RH");
    Serial.println("--------------------");
  } else {
    Serial.println("Failed to read gas data!");
  }

  delay(1000);
}
