/*!
 * @file setOperationMode.ino
 * @brief Set and read the Gravity SGX6410 operation mode
 * @details Switch the sensor to PBAQ mode, then print the mode register and TVOC/ETOH.
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

  Serial.println("Gravity SGX6410 setOperationMode");

  while (sensor.begin() != true) {
    Serial.println("Init failed, check I2C address and wiring");
    delay(1000);
  }

  while (sensor.setOperationMode(DFRobot_SGX6410_Gravity::ePbaq) != true) {
    Serial.println("Set PBAQ mode failed");
    delay(1000);
  }
  Serial.print("Mode=");
  Serial.println((uint8_t)sensor.getOperationMode(), HEX);
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
