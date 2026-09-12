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

  // Firmware reads SHT40 only after automatic compensation is enabled
  while (sgx6410.setAutoHumiCompensation(DFRobot_SGX6410_Gravity::eHumCompEnable) != true) {
    Serial.println("Enable humidity compensation failed");
    delay(1000);
  }

  Serial.println("======== Gravity SGX6410 getSHT40Data ========");
  Serial.print("Init      : ");
  Serial.println("OK");
  Serial.print("HumComp   : ");
  Serial.println("ON");
  Serial.println("===============================================");
}

void loop()
{
  // Temperature = -45 + 175 * raw / 65535, humidity = -6 + 125 * raw / 65535
  DFRobot_SGX6410_Gravity::sSHT40Data_t &sht40 = sgx6410.getSHT40Data();
  // NAN until the bridge has cached the first SHT40 sample
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
