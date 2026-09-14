# DFRobot_SGX6410_Gravity
- [English Version](./README.md)

DFRobot_SGX6410_Gravity 是 Gravity MEMS 空气质量传感器（SKU: SEN0771）的 Arduino 库。模块基于 SGX6410 气体传感器评估室内空气质量，可输出 IAQ、TVOC、乙醇当量（ETOH）、估计 CO2（eCO2）和相对 IAQ（RelIAQ）。板载 SHT40 可提供温湿度，用于自动湿度补偿。

主机通过 I2C 或 UART Modbus RTU 与 Gravity 模块通信，不会在总线上直接访问 SGX6410 或 SHT40。I2C 使用 16 位小端寄存器表；UART 默认 9600 8N1。从机地址为 `0x52` 或 `0x53`，由 ADD_SEL 在上电时锁存；COM_SEL 用于选择 I2C 或 UART。

典型流程：调用 `begin()` 校验 DFRobot 厂商 ID（`0x3343`），设置工作模式（IAQ / ULP / PBAQ / Suspend），等待预热完成（`valid` 为 true），再调用 `update()` 读取缓存的气体数据。湿度可按 0–100 %RH 手动写入，也可在打开自动补偿后由模块按约 60 s 周期用 SHT40 更新。

## 产品链接

```
SKU: SEN0771
```

## 目录

  * [概述](#概述)
  * [库安装](#库安装)
  * [方法](#方法)
  * [兼容性](#兼容性)
  * [历史](#历史)
  * [创作者](#创作者)

## 概述

* 双接口：I2C（16 位小端寄存器）或 UART Modbus RTU（保持寄存器 `0x03`/`0x06`，输入寄存器 `0x04`），地址 `0x52` / `0x53`
* UART 默认 9600 8N1。ADD_SEL 在上电时锁存，UART 模式下同时作为 Modbus 从机地址
* `begin()` 校验 DFRobot 厂商 ID `0x3343`，随后可读取模块 PID、固件版本、锁存地址、SGX6410 产品 ID（`0x2310`）和 6 字节序列号
* 工作模式：Suspend；IAQ（约 3 s 采样，预热约 5 min）；ULP（约 90 s 采样，预热约 15 min）；PBAQ（约 5 s 采样，预热约 5 min）
* `update()` 按模式换算：IAQ/ULP 为 IAQ=raw/10、TVOC=raw/100 mg/m³、ETOH=raw/100 ppm、eCO2=raw ppm、RelIAQ=raw/10；PBAQ 仅输出 TVOC/ETOH（raw/1000），其余为 NAN
* 手动湿度补偿：写入 0–100 %RH（内部存为 0–255 码）。打开自动补偿后，模块约每 60 s 用 SHT40 刷新该值
* 自动补偿打开后，`getSHT40Data()` 可读取板载温度（°C）和湿度（%RH）
* 一个生命周期一次的热清洁通过 `runSensorClean()` 执行（约 60 s），清洁过程中请保持供电稳定

## 库安装

* 在 Arduino IDE 库管理器中搜索 `DFRobot_SGX6410_Gravity` 并安装。
* 或下载本仓库，解压到 Arduino 的 `libraries` 目录，再打开 `examples` 中的示例。
* UART / Modbus RTU 还需要安装 `DFRobot_RTU` 库。

## 方法

```C++
  /**
   * @fn DFRobot_SGX6410_Gravity_UART
   * @brief 构造函数（UNO/ESP8266 使用 SoftwareSerial）
   * @param sSerial SoftwareSerial 对象指针
   * @param baud 波特率，默认 9600
   * @param addr Modbus 从机地址，0x52 或 0x53，默认 0x53
   * @n ADD_SEL 低电平选择 0x52，高电平选择 0x53。拨码在模块上电时锁存。
   * @n COM_SEL 必须拨到 UART。默认串口参数为 9600 8N1。
   */
  DFRobot_SGX6410_Gravity_UART(SoftwareSerial *sSerial, uint32_t baud = SGX_UART_BAUD_DEFAULT, uint8_t addr = SGX_ADDR_DEFAULT);

  /**
   * @fn DFRobot_SGX6410_Gravity_UART
   * @brief 构造函数（HardwareSerial）
   * @param hSerial HardwareSerial 对象指针，通常为 &Serial1
   * @param baud 波特率，默认 9600
   * @param addr Modbus 从机地址，0x52 或 0x53，默认 0x53
   * @param rxPin RX 引脚，0 表示使用板级默认 RX
   * @param txPin TX 引脚，0 表示使用板级默认 TX
   * @n ADD_SEL 低电平选择 0x52，高电平选择 0x53。拨码在模块上电时锁存。
   * @n COM_SEL 必须拨到 UART。默认串口参数为 9600 8N1。
   * @n ESP32：传入 rxPin/txPin 可改 Serial1 引脚，省略则用板级默认脚。
   */
  DFRobot_SGX6410_Gravity_UART(HardwareSerial *hSerial, uint32_t baud = SGX_UART_BAUD_DEFAULT, uint8_t addr = SGX_ADDR_DEFAULT, uint8_t rxPin = 0, uint8_t txPin = 0);

  /**
   * @fn begin
   * @brief 初始化 UART 并校验模块
   * @return bool
   * @retval true  初始化成功
   * @retval false 初始化失败
   * @n 按构造函数波特率打开注入的串口，设置 Modbus RTU 超时，再执行基类 begin() 的 VID 检查。
   * @n ESP32 会在此处应用构造函数里的自定义 RX/TX。
   */
  bool begin(void);

  /**
   * @fn DFRobot_SGX6410_Gravity_I2C
   * @brief 构造函数
   * @param pWire I2C 对象指针，通常为 &Wire
   * @param addr 7 位 I2C 地址，0x52 或 0x53，默认 0x53
   * @param sclPin SCL 引脚，默认 SGX_I2C_PIN_DEFAULT
   * @param sdaPin SDA 引脚，默认 SGX_I2C_PIN_DEFAULT
   * @n ADD_SEL 低电平选择 0x52，高电平选择 0x53。拨码在模块上电时锁存。
   * @n ESP32/ESP8266：传入 sclPin/sdaPin 可改 I2C 引脚，省略则用板级默认脚。
   * @n UNO 等固定 Wire 引脚的主板：不要传 sclPin/sdaPin，begin() 会忽略这两个参数。
   */
  DFRobot_SGX6410_Gravity_I2C(TwoWire *pWire, uint8_t addr = SGX_ADDR_DEFAULT, uint8_t sclPin = SGX_I2C_PIN_DEFAULT, uint8_t sdaPin = SGX_I2C_PIN_DEFAULT);

  /**
   * @fn begin
   * @brief 初始化 I2C 并校验模块
   * @return bool
   * @retval true  初始化成功
   * @retval false 初始化失败
   * @n 对注入的总线调用 TwoWire::begin()，再执行基类 begin() 的 VID 检查。
   * @n ESP32/ESP8266 会在此处应用构造函数里的自定义 SCL/SDA。UNO 始终使用默认 Wire 引脚。
   */
  bool begin(void);

  /**
   * @fn begin
   * @brief 初始化模块并校验厂商 ID
   * @return bool
   * @retval true  初始化成功
   * @retval false 初始化失败
   * @n 读取 DEVICE_VID（0x3343）。模块 PID 在产品 ID 确定前可能仍为 0x0000。
   */
  bool begin(void);

  /**
   * @fn reset
   * @brief 向模块发送传感器复位命令
   * @return bool
   * @retval true  写入成功
   * @retval false 写入失败
   * @n 主机向复位寄存器写入 0x0001。该寄存器读回为 0。
   * @n 复位后需等待模块完成 SGX6410 启动，再调用 update()。
   */
  bool reset(void);

  /**
   * @fn setOperationMode
   * @brief 设置 SGX6410 工作模式
   * @param mode 工作模式（见 eMode_t）
   * @n eSuspend: 休眠模式
   * @n eIaq: IAQ 第二代，采样周期约 3 s，预热 5 min
   * @n eUlp: 超低功耗，采样周期约 90 s，预热 15 min
   * @n ePbaq: PBAQ，采样周期约 5 s，预热 5 min
   * @n 改模式属于维护操作。模块会重新预热并清除 isNew/valid。
   * @n 传感器清洁 0x80 应通过 runSensorClean() 启动，不要反复调用。
   * @return bool
   * @retval true  设置成功
   * @retval false 设置失败或模式不允许
   */
  bool setOperationMode(eMode_t mode);

  /**
   * @fn runSensorClean
   * @brief 执行传感器热清洁：启动清洁并轮询完成状态
   * @param timeoutMs 超时毫秒，建议 >= 90000（清洁约 60 s）
   * @return uint8_t CLEAN_OK=0 清洁完成 / CLEAN_DONE=1 已清洁过 / CLEAN_TIMEOUT=2 超时 / CLEAN_FAIL=3 通讯失败
   * @warning 清洁中断可能永久损伤气敏材料，请保证供电稳定。整个生命周期建议只清洁一次。
   * @n 先读 REG_I2C_CLEAN_FLAG，为 1 则直接返回 CLEAN_DONE，不发送 0x80。
   * @n 否则写入 eSensorClean（0x80），轮询模式寄存器：0x80 仍在清洁，0x00 清洁完毕。
   */
  uint8_t runSensorClean(uint32_t timeoutMs);

  /**
   * @fn getOperationMode
   * @brief 读取当前 SGX6410 工作模式
   * @return eMode_t 当前模式寄存器值
   * @n 读取失败时返回 eSuspend。
   */
  eMode_t getOperationMode(void);

  /**
   * @fn setHumidityCompensate
   * @brief 写入环境相对湿度，供算法补偿
   * @param rh 相对湿度，范围 0.0 到 100.0，单位 %RH
   * @n 主机将 rh 转为 code = round(rh * 255 / 100)，写入 0-255 湿度码。
   * @n 这是手动写入，不会打开板载 SHT40 通道。
   * @n 若自动补偿已打开，模块可能在下一次 SHT40 周期覆盖该值。
   * @return bool
   * @retval true  写入成功
   * @retval false rh 超范围或写入失败
   */
  bool setHumidityCompensate(float rh);

  /**
   * @fn getHumidityCompensate
   * @brief 将最近一次湿度码读回为 %RH
   * @return float 由 0-255 湿度码还原的相对湿度 %RH
   * @n 寄存器读取失败时返回 NAN。
   * @n 这是最近写入 SGX6410 的值，不是 SHT40 的实时采样。
   */
  float getHumidityCompensate(void);

  /**
   * @fn setAutoHumiCompensation
   * @brief 打开或关闭模块上的 SHT40 自动湿度补偿
   * @param enable 补偿开关（见 eHumComp_t）
   * @n eHumCompDisable: 停止读取 SHT40；SGX6410 保留最后一次湿度码
   * @n eHumCompEnable: 立即读一次 SHT40，之后约每 60 s 一次
   * @n 关闭补偿不会自动恢复 SGX6410 默认 50 %RH，除非再用 setHumidityCompensate() 写入。
   * @return bool
   * @retval true  写入成功
   * @retval false 非法值或写入失败
   */
  bool setAutoHumiCompensation(eHumComp_t enable);

  /**
   * @fn getHumidityCompEnable
   * @brief 读取自动湿度补偿开关
   * @return eHumComp_t 当前开关
   * @n 读取失败时返回 eHumCompDisable。
   */
  eHumComp_t getHumidityCompEnable(void);

  /**
   * @fn getVid
   * @brief 读取 DFRobot 厂商 ID
   * @return uint16_t 厂商 ID，期望为 0x3343
   * @n 读取失败时返回 0。
   */
  uint16_t getVid(void);

  /**
   * @fn getPid
   * @brief 读取模块产品 ID
   * @return uint16_t 模块 PID
   * @n 固件在产品 ID 确定前当前上报 0x0000。
   * @n 读取失败时返回 0。
   */
  uint16_t getPid(void);

  /**
   * @fn getFwVersion
   * @brief 读取模块固件版本
   * @return uint16_t 编码版本，V1.0.0.0 为 0x1000
   * @n 读取失败时返回 0。
   */
  uint16_t getFwVersion(void);

  /**
   * @fn getDeviceAddr
   * @brief 读取已锁存的外部从机地址
   * @return uint8_t 7 位地址，0x52 或 0x53
   * @n 拨码 ADD_SEL 在上电时采样，UART 模式下同时作为 Modbus 从机地址。
   * @n 运行中拨动开关需复位后才生效。
   * @n 读取失败时返回 0。
   */
  uint8_t getDeviceAddr(void);

  /**
   * @fn getSgxProductId
   * @brief 读取缓存的 SGX6410 产品 ID
   * @return uint16_t 传感器产品 ID，期望为 0x2310
   * @n 读取失败或传感器尚未识别时返回 0。
   */
  uint16_t getSgxProductId(void);

  /**
   * @fn getTrackingNumber
   * @brief 读取 6 字节 SGX6410 序列号
   * @param number 调用方提供的 6 字节输出缓冲
   * @return bool
   * @retval true  读取成功
   * @retval false 空指针或读取失败
   */
  bool getTrackingNumber(uint8_t *number);

  /**
   * @fn update
   * @brief 读取模式标志和测量寄存器，并换算为工程单位
   * @return bool
   * @retval true  寄存器已读到（重复样本仍返回 true，isNew = false）
   * @retval false 总线错误或当前模式没有测量数据
   * @n 调用 getAllGasData() / getSingleGasData() 前必须先调用本函数。
   * @n IAQ/ULP：IAQ = raw/10，TVOC = raw/100 mg/m3，ETOH = raw/100 ppm，ECO2 = raw ppm，RELIAQ = raw/10
   * @n PBAQ：TVOC = raw/1000 mg/m3，ETOH = raw/1000 ppm；IAQ、ECO2、RELIAQ 为 NAN
   * @n isNew 与 valid 由模块给出。读到新样本后，主机将 isNew 写回 0。
   */
  bool update(void);

  /**
   * @fn getAllGasData
   * @brief 返回缓存的气体测量结构体
   * @return const sGasData_t & 最近一次缓存的测量
   * @n 须先调用 update()。用 isNew 判断是否新样本，用 valid 判断预热是否完成。
   */
  const sGasData_t &getAllGasData(void) const;

  /**
   * @fn getSingleGasData
   * @brief 从缓存中返回单个气体值
   * @param dataType 由 eDataType_t 选择的气体类型
   * @n eDataIaq: IAQ
   * @n eDataTvoc: TVOC
   * @n eDataEtoh: ETOH
   * @n eDataEco2: 估计 CO2
   * @n eDataRelIaq: 相对 IAQ
   * @return float 缓存值；当前模式不支持时为 NAN
   * @n 须先调用 update()。
   */
  float getSingleGasData(eDataType_t dataType);

  /**
   * @fn getSHT40Data
   * @brief 读取板载 SHT40 温湿度（Gravity 桥接缓存）
   * @return sSHT40Data_t & 缓存的温度（C）和湿度（%RH）
   * @n Gravity 版本专用 API。主机不直接访问 SHT40。
   * @n 固件仅在 setAutoHumiCompensation(eHumCompEnable) 之后才会读取 SHT40。
   * @n 若补偿关闭，或尚未缓存到有效样本，temperature 和 humidity 为 NAN。
   * @n 温度 = -45 + 175 * raw / 65535。湿度 = -6 + 125 * raw / 65535。
   */
  sSHT40Data_t &getSHT40Data(void);
```

## 兼容性

| 主板         | 通过 | 未通过 | 未测试 | 备注 |
| ------------ | :--: | :----: | :----: | ---- |
| Arduino uno  |      |        |   √    |      |
| Mega2560     |      |        |   √    |      |
| Leonardo     |      |        |   √    |      |
| ESP32        |      |        |   √    |      |
| ESP8266      |      |        |   √    |      |
| micro:bit    |      |        |   √    |      |

## 历史

- 2026/09/09 - V1.0.0 版本

## 创作者

Written by PLELES(li.jia@dfrobot.com), 2026. (欢迎访问我们的[网站](https://www.dfrobot.com/))
