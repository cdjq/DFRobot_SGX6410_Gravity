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

// Select operation mode: eIaq / eUlp / ePbaq / eSuspend
#define SGX6410_OPERATION_MODE DFRobot_SGX6410_Gravity::ePbaq

/* Select one communication interface (uncomment exactly one) */
// #define SGX6410_COMM_UART
#define SGX6410_COMM_I2C

const uint8_t deviceAddr = SGX_ADDR_DEFAULT;    // ADD_SEL low = 0x52, ADD_SEL high = 0x53

#if defined(SGX6410_COMM_UART)
/* ---------------------------------------------------------------------------------------------------------
 * Hardware connection table:
 *    Sensor Pin |        MCU Pin        | Leonardo/Mega2560/M0 |    UNO    | ESP8266 | ESP32 |
 *     VCC       |          5V           |         5V           |    5V     |   5V    |  5V   |
 *     GND       |         GND           |        GND           |    GND    |   GND   |  GND  |
 *     RX        |        MCU TX         |     Serial1 TX1      |     3     |  GPIO5  |  17   |
 *     TX        |        MCU RX         |     Serial1 RX1      |     2     |  GPIO4  |  16   |
 * ---------------------------------------------------------------------------------------------------------*/
/* Baud rate must match the module (default 9600 8N1). SoftSerial may be unstable at high baud rates. */
/* COM_SEL must be UART. */
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
#include <SoftwareSerial.h>
#if defined(ARDUINO_AVR_UNO)
SoftwareSerial mySerial(2, 3);    // RX=2, TX=3
#elif defined(ESP8266)
SoftwareSerial mySerial(4, 5);    // RX=GPIO4, TX=GPIO5
#endif
DFRobot_SGX6410_Gravity_UART sgx6410(&mySerial, SGX_UART_BAUD_DEFAULT, deviceAddr);
#elif defined(ESP32)
DFRobot_SGX6410_Gravity_UART sgx6410(&Serial1, SGX_UART_BAUD_DEFAULT, deviceAddr, /*RX pin*/ 16, /*TX pin*/ 17);
#else
DFRobot_SGX6410_Gravity_UART sgx6410(&Serial1, SGX_UART_BAUD_DEFAULT, deviceAddr);
#endif
#elif defined(SGX6410_COMM_I2C)
/* ---------------------------------------------------------------------------------------------------------
 *   SGX6410  |      MCU       | Leonardo/Mega2560/M0 | UNO | ESP8266 | ESP32 | Other Wire boards
 *     VCC    |      3.3V      |        3.3V           | 3.3V|   3.3V  | 3.3V  |        3.3V
 *     GND    |      GND       |        GND            | GND |   GND   | GND   |        GND
 *     SCL    |      SCL       |        SCL            | SCL |  GPIO5  | SCL   |        SCL
 *     SDA    |      SDA       |        SDA            | SDA |  GPIO4  | SDA   |        SDA
 * ---------------------------------------------------------------------------------------------------------*/
/**
 * I2C address: ADD_SEL low = 0x52, ADD_SEL high = 0x53 (default)
 * ESP32/ESP8266 can also pass SCL/SDA. UNO should omit pin arguments.
 * COM_SEL must be I2C.
 */
DFRobot_SGX6410_Gravity_I2C sgx6410(&Wire, deviceAddr);
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

  // Apply the operation mode selected above
  while (sgx6410.setOperationMode(SGX6410_OPERATION_MODE) != true) {
    Serial.println("Set operation mode failed");
    delay(1000);
  }

  Serial.println("======== Gravity SGX6410 setOperationMode ========");
  Serial.print("Init      : ");
  Serial.println("OK");
  Serial.print("Mode      : 0x");
  Serial.println((uint8_t)sgx6410.getOperationMode(), HEX);
  Serial.println("===================================================");
}

void loop()
{
  // update() reads the measurement registers and scales them
  if (sgx6410.update()) {
    const DFRobot_SGX6410_Gravity::sGasData_t &data = sgx6410.getAllGasData();
    // Skip if the bridge has not published a new sample
    if (!data.isNew) {
      delay(500);
      return;
    }
    // PBAQ leaves IAQ / ECO2 / RELIAQ as NAN
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
    Serial.println("--------------------");
  } else {
    Serial.println("Failed to read gas data!");
  }

  delay(1000);
}
