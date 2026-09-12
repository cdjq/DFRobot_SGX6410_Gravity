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

/* Select one communication interface (uncomment exactly one) */
#define SGX6410_COMM_UART
// #define SGX6410_COMM_I2C

const uint8_t DEVICE_ADDR = SGX_ADDR_DEFAULT;    // ADD_SEL low = 0x52, ADD_SEL high = 0x53

#if defined(SGX6410_COMM_UART)
/* ---------------------------------------------------------------------------------------------------------
 * Hardware connection table:
 *    Sensor Pin |        MCU Pin        | Leonardo/Mega2560/M0 |    UNO    | ESP8266 | ESP32 |  microbit  |
 *     VCC       |          5V           |         5V           |    5V     |   5V    |  5V   |     X      |
 *     GND       |         GND           |        GND           |    GND    |   GND   |  GND  |     X      |
 *     RX        |        MCU TX         |     Serial1 TX1      |     3     |    3    |  17   |     X      |
 *     TX        |        MCU RX         |     Serial1 RX1      |     2     |    2    |  16   |     X      |
 * ---------------------------------------------------------------------------------------------------------*/
/* Baud rate must match the module (default 9600 8N1). SoftSerial may be unstable at high baud rates. */
/* COM_SEL must be UART. */
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);    // RX=2, TX=3
DFRobot_SGX6410_Gravity_UART sgx6410(&mySerial, SGX_UART_BAUD_DEFAULT, DEVICE_ADDR);
#elif defined(ESP32)
DFRobot_SGX6410_Gravity_UART sgx6410(&Serial1, SGX_UART_BAUD_DEFAULT, DEVICE_ADDR, /*RX pin*/ 16, /*TX pin*/ 17);
#elif defined(ARDUINO_BBC_MICROBIT) && !defined(ARDUINO_BBC_MICROBIT_V2)
#error "BBC micro:bit has no usable USART/Serial1. Use I2C."
#else
DFRobot_SGX6410_Gravity_UART sgx6410(&Serial1, SGX_UART_BAUD_DEFAULT, DEVICE_ADDR);
#endif
#elif defined(SGX6410_COMM_I2C)
/**
 * I2C address: ADD_SEL low = 0x52, ADD_SEL high = 0x53 (default)
 * ESP32/ESP8266 can also pass SCL/SDA. UNO should omit pin arguments.
 * COM_SEL must be I2C.
 */
DFRobot_SGX6410_Gravity_I2C sgx6410(&Wire, DEVICE_ADDR);
#else
#error "Please define SGX6410_COMM_UART or SGX6410_COMM_I2C (exactly one)"
#endif

void setup()
{
  uint8_t result;

  // Open the debug serial port
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  // Initialize the module and verify VID 0x3343
  while (sgx6410.begin() != true) {
    Serial.println("Init failed, check address, COM_SEL and wiring");
    delay(1000);
  }

  // Clean once in the sensor lifetime. Keep power stable for about 60 s
  result = sgx6410.runSensorClean(90000);

  Serial.println("======== Gravity SGX6410 runSensorClean ========");
  Serial.print("Init      : ");
  Serial.println("OK");
  Serial.print("Clean     : ");
  if (result == CLEAN_OK) {
    Serial.println("finished");
  } else if (result == CLEAN_DONE) {
    Serial.println("already cleaned, skip");
  } else if (result == CLEAN_TIMEOUT) {
    Serial.println("timeout, do not power off if still cleaning");
  } else {
    Serial.println("failed, check communication");
  }
  Serial.print("Note      : ");
  Serial.println("Keep power stable. Clean takes about 60 s. Run only once.");
  Serial.println("==================================================");
}

void loop()
{
  // Clean runs only in setup()
  delay(1000);
}
