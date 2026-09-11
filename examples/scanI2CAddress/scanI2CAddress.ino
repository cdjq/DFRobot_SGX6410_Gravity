/*!
 * @file scanI2CAddress.ino
 * @brief Scan the I2C bus for Gravity SGX6410
 * @details Probe 7-bit addresses 0x01..0x7F and print every ACK.
 * @n ADD_SEL low = 0x52, ADD_SEL high = 0x53 (default).
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author PLELES(li.jia@dfrobot.com)
 * @version V1.0.0
 * @date 2026-09-10
 * @url https://github.com/DFRobot/DFRobot_SGX6410_Gravity
 */

#include "Wire.h"

/**
 * ESP32/ESP8266: pass SCL/SDA to Wire.begin() if you remapped the pins.
 * UNO and other boards with fixed Wire pins: keep Wire.begin().
 */
void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Wire.begin();
  // Wire.begin(SDA, SCL);
#if defined(ARDUINO_ARCH_ESP32)
  Wire.setTimeOut(100);
#elif defined(ARDUINO_ARCH_ESP8266)
  Wire.setClockStretchLimit(150000);
#elif defined(WIRE_HAS_TIMEOUT)
  Wire.setWireTimeout(25000, true);
#endif

  Serial.println("Gravity SGX6410 I2C scan");
  Serial.println("Expected: 0x52 (ADD_SEL low) or 0x53 (ADD_SEL high)");
}

void loop()
{
  uint8_t found = 0;

  Serial.println("Scanning...");
  Serial.flush();
  for (uint8_t addr = 1; addr < 127; addr++) {
    Serial.print("Probe 0x");
    if (addr < 0x10) {
      Serial.print("0");
    }
    Serial.print(addr, HEX);
    Serial.flush();

    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    Serial.print("  err=");
    Serial.println(err);

    if (err == 0) {
      found++;
      Serial.print("Found 0x");
      if (addr < 0x10) {
        Serial.print("0");
      }
      Serial.print(addr, HEX);
      if (addr == 0x52) {
        Serial.print("  (SGX6410 ADD_SEL low)");
      } else if (addr == 0x53) {
        Serial.print("  (SGX6410 ADD_SEL high)");
      }
      Serial.println();
    }
  }

  if (found == 0) {
    Serial.println("No I2C device found, check wiring and COM_SEL");
  } else {
    Serial.print("Devices found: ");
    Serial.println(found);
  }
  Serial.println("--------------------");
  delay(2000);
}
