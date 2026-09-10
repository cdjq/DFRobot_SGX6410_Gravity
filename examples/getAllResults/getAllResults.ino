/*!
 * @file getAllResults.ino
 * @brief Read Gravity SGX6410 air quality data over I2C
 * @details Initialize the module, set IAQ mode, enable automatic humidity compensation,
 * @n then poll scaled IAQ/TVOC/ETOH/eCO2/RelIAQ values.
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

  Serial.println("Gravity SGX6410 getAllResults");

  while (sensor.begin() != true) {
    Serial.println("Init failed, check I2C address and wiring");
    delay(1000);
  }
  Serial.println("Init success");

  Serial.print("VID=0x");
  Serial.println(sensor.getVid(), HEX);
  Serial.print("SGX PID=0x");
  Serial.println(sensor.getSgxProductId(), HEX);

  uint8_t tracking[6] = {0, 0, 0, 0, 0, 0};
  if (sensor.getTrackingNumber(tracking)) {
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

  while (sensor.setOperationMode(DFRobot_SGX6410_Gravity::eIaq) != true) {
    Serial.println("Set IAQ mode failed");
    delay(1000);
  }
  Serial.println("IAQ mode set");

  while (sensor.setHumidityCompEnable(DFRobot_SGX6410_Gravity::eHumCompEnable) != true) {
    Serial.println("Enable humidity compensation failed");
    delay(1000);
  }
  Serial.println("Humidity compensation enabled");
}

void loop()
{
  if (sensor.update()) {
    const DFRobot_SGX6410_Gravity::sGasData_t &data = sensor.getAllGasData();
    if (!data.isNew) {
      delay(500);
      return;
    }
    Serial.print("IAQ:      ");
    Serial.println(data.IAQ, 2);
    Serial.print("TVOC:     ");
    Serial.print(data.TVOC, 2);
    Serial.println(" mg/m3");
    Serial.print("ETOH:     ");
    Serial.print(data.ETOH, 2);
    Serial.println(" ppm");
    Serial.print("ECO2:     ");
    Serial.print((uint16_t)data.ECO2);
    Serial.println(" ppm");
    Serial.print("RELIAQ:   ");
    Serial.println(data.RELIAQ, 2);
    Serial.print("Data valid: ");
    Serial.println(data.valid ? "YES" : "NO (warming up)");
    Serial.println("--------------------");
  } else {
    Serial.println("Failed to read gas data!");
  }

  delay(1000);
}
