/*!
 * @file getData.ino
 * @brief Read Gravity SGX6410 air quality data without humidity compensation
 * @details Initialize the module, set IAQ mode, then poll scaled IAQ/TVOC/ETOH/eCO2/RelIAQ.
 * @n Humidity compensation is not enabled, so SHT40 humidity is not printed.
 * @n The humidity compensation switch is still read and printed.
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
  uint8_t tracking[6] = {0, 0, 0, 0, 0, 0};

  // Open the debug serial port
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("======== Gravity SGX6410 getData ========");
  Serial.flush();

  // Initialize the module and verify VID 0x3343
  while (sgx6410.begin() != true) {
    Serial.println("Init failed, check address, COM_SEL and wiring");
    Serial.flush();
    delay(1000);
  }

  // IAQ: about 3 s sample period, 5 min warm-up
  while (sgx6410.setOperationMode(DFRobot_SGX6410_Gravity::eIaq) != true) {
    Serial.println("Set IAQ mode failed");
    delay(1000);
  }
  // This demo does not use automatic SHT40 compensation
  // while(sgx6410.setAutoHumiCompensation(HUM_COMP_DISABLE) != true) {
  //   Serial.println("close humidity compensation failed");
  //   delay(1000);
  // }

  // Print module identity after init
  Serial.print("Init      : ");
  Serial.println("OK");
  Serial.print("VID       : 0x");
  Serial.println(sgx6410.getVid(), HEX);
  Serial.print("SGX PID   : 0x");
  Serial.println(sgx6410.getSgxProductId(), HEX);
  Serial.print("Tracking  : ");
  if (sgx6410.getTrackingNumber(tracking)) {
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
  } else {
    Serial.println("--");
  }
  Serial.print("Mode      : ");
  Serial.println("IAQ");
  Serial.println("==========================================");
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
    // Print gas data. valid stays false during warm-up
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
    Serial.println("--------------------");
  } else {
    Serial.println("Failed to read gas data!");
  }
  delay(1000);
}
