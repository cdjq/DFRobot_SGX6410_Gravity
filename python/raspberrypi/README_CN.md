# DFRobot_SGX6410_Gravity
- [English Version](./README.md)

DFRobot_SGX6410_Gravity 是 Gravity MEMS 空气质量传感器（SKU: SEN0771）的树莓派 Python 库。模块基于 SGX6410 气体传感器评估室内空气质量，可输出 IAQ、TVOC、乙醇当量（ETOH）、估计 CO2（eCO2）和相对 IAQ（RelIAQ）。板载 SHT40 可提供温湿度，用于自动湿度补偿。

主机通过 I2C 或 UART Modbus RTU 与 Gravity 模块通信，不会在总线上直接访问 SGX6410 或 SHT40。I2C 使用 16 位小端寄存器表（树莓派通常为总线 1）；UART 默认 9600 8N1，串口为 `/dev/ttyAMA0`。从机地址为 `0x52` 或 `0x53`，由 ADD_SEL 在上电时锁存；COM_SEL 用于选择 I2C 或 UART。

典型流程：调用 `begin()` 校验 DFRobot 厂商 ID（`0x3343`），设置工作模式（IAQ / ULP / PBAQ / Suspend），等待预热完成（`valid` 为 true），再调用 `update()` 读取缓存的气体数据。湿度可按 0–100 %RH 手动写入，也可在打开自动补偿后由模块按约 60 s 周期用 SHT40 更新。示例里 `ctype = 0` 为 I2C，非 0 为 UART。

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

* 双接口：I2C（`SMBus`）或 UART Modbus RTU（`/dev/ttyAMA0`，保持寄存器 `0x03`/`0x06`，输入寄存器 `0x04`），地址 `0x52` / `0x53`
* UART 默认 9600 8N1。ADD_SEL 在上电时锁存，UART 模式下同时作为 Modbus 从机地址
* `begin()` 校验 DFRobot 厂商 ID `0x3343`，随后可读取模块 PID、固件版本、锁存地址、SGX6410 产品 ID（`0x2310`）和 6 字节序列号
* 工作模式：Suspend；IAQ（约 3 s 采样，预热约 5 min）；ULP（约 90 s 采样，预热约 15 min）；PBAQ（约 5 s 采样，预热约 5 min）
* `update()` 按模式换算：IAQ/ULP 为 IAQ=raw/10、TVOC=raw/100 mg/m³、ETOH=raw/100 ppm、eCO2=raw ppm、RelIAQ=raw/10；PBAQ 仅输出 TVOC/ETOH（raw/1000），其余为 NAN
* 使用数据前请检查 `is_new`（是否新样本）和 `valid`（预热是否完成）
* 手动湿度补偿：写入 0–100 %RH（内部存为 0–255 码）。打开自动补偿后，模块约每 60 s 用 SHT40 刷新该值
* 自动补偿打开后，`get_sht40_data()` 可读取板载温度（°C）和湿度（%RH）
* 一生一次的热清洁通过 `run_sensor_clean()` 执行（约 60 s），清洁过程中请保持供电稳定

## 库安装

1. 在树莓派上打开 I2C 和/或 UART（`raspi-config`）。
2. 安装依赖：

```
sudo apt-get update
sudo apt-get install python3-smbus python3-serial
```

3. 把本目录 `python/raspberrypi/` 拷到树莓派，然后运行示例：

```
python3 examples/get_data/get_data.py
```

`ctype = 0` 为 I2C，非 0 为 UART。UART 需要与 `DFRobot_SGX6410_Gravity.py` 同级的 `DFRobot_RTU.py`。

## 方法

```python
  def __init__(self, bus, addr)
    '''!
      @brief 构造函数
      @param bus I2C 总线号，树莓派通常为 1
      @param addr 从机地址，0x52 或 0x53，默认 0x53
      @n ADD_SEL 低电平选择 0x52，高电平选择 0x53。拨码在模块上电时锁存。
    '''

  def __init__(self, baud, addr)
    '''!
      @brief 构造函数
      @param baud 波特率，默认 9600
      @param addr Modbus 从机地址，0x52 或 0x53，默认 0x53
      @n COM_SEL 必须拨到 UART。默认串口为 /dev/ttyAMA0，9600 8N1。
    '''

  def begin(self)
    '''!
      @brief 初始化模块并校验厂商 ID
      @return 成功 True，失败 False
      @n 读取 DEVICE_VID（0x3343）。模块 PID 在产品 ID 确定前可能仍为 0x0000。
    '''

  def reset(self)
    '''!
      @brief 向模块发送传感器复位命令
      @return 成功 True，失败 False
      @n 向复位寄存器写入 0x0001，读回为 0。
      @n 复位后需等待模块完成 SGX6410 启动再调用 update()。
    '''

  def set_operation_mode(self, mode)
    '''!
      @brief 设置 SGX6410 工作模式
      @param mode 工作模式
      @n SUSPEND：休眠
      @n IAQ：约 3 s 采样，预热 5 min
      @n ULP：约 90 s 采样，预热 15 min
      @n PBAQ：约 5 s 采样，预热 5 min
      @n 改模式会重新预热并清除 is_new/valid。
      @n 清洁模式 0x80 请用 run_sensor_clean()，不要反复调用。
      @return 成功 True，失败 False
    '''

  def run_sensor_clean(self, timeout_ms)
    '''!
      @brief 执行传感器热清洁：启动后轮询直到完成
      @param timeout_ms 超时时间，单位 ms，建议 >= 90000（清洁约 60 s）
      @return CLEAN_OK=0 完成 / CLEAN_DONE=1 已清洁过 / CLEAN_TIMEOUT=2 超时 / CLEAN_FAIL=3 通信失败
      @n 一生只清洁一次。
    '''

  def get_operation_mode(self)
    '''!
      @brief 读取当前 SGX6410 工作模式
      @return 当前模式寄存器值
      @n 读取失败时返回 SUSPEND。
    '''

  def set_humidity_compensate(self, rh)
    '''!
      @brief 写入手动湿度补偿
      @param rh 相对湿度，范围 0.0 到 100.0，单位 %RH
      @n 主机换算 code = round(rh * 255 / 100) 后写入 0-255。
      @n 这是手动写入，不会打开板载 SHT40 通路。
      @return 成功 True，失败 False
    '''

  def get_humidity_compensate(self)
    '''!
      @brief 读取最近一次写入的湿度码，换算为 %RH
      @return 相对湿度 %RH
      @n 读取失败时返回 NAN。
    '''

  def set_auto_humi_compensation(self, enable)
    '''!
      @brief 打开或关闭模块上的 SHT40 自动湿度补偿
      @param enable HUM_COMP_DISABLE 或 HUM_COMP_ENABLE
      @n HUM_COMP_ENABLE：立即读一次 SHT40，之后约每 60 s 一次
      @return 成功 True，失败 False
    '''

  def get_humidity_comp_enable(self)
    '''!
      @brief 读取自动湿度补偿开关
      @return 当前开关
      @n 读取失败时返回 HUM_COMP_DISABLE。
    '''

  def get_vid(self)
    '''!
      @brief 读取 DFRobot 厂商 ID
      @return 厂商 ID，期望为 0x3343
    '''

  def get_pid(self)
    '''!
      @brief 读取模块产品 ID
      @return 模块 PID
    '''

  def get_fw_version(self)
    '''!
      @brief 读取模块固件版本
      @return 编码版本，V1.0.0.0 为 0x1000
    '''

  def get_device_addr(self)
    '''!
      @brief 读取已锁存的外部从机地址
      @return 7 位地址，0x52 或 0x53
    '''

  def get_sgx_product_id(self)
    '''!
      @brief 读取缓存的 SGX6410 产品 ID
      @return 传感器产品 ID，期望为 0x2310
    '''

  def get_tracking_number(self)
    '''!
      @brief 读取 6 字节 SGX6410 序列号
      @return 成功返回 6 字节列表，失败返回 None
    '''

  def update(self)
    '''!
      @brief 读取模式标志和测量寄存器，并换算为工程单位
      @return 寄存器读到则 True
      @n 调用 get_all_gas_data() / get_single_gas_data() 前必须先调用本函数。
    '''

  def get_all_gas_data(self)
    '''!
      @brief 返回缓存的气体测量结构
      @return GasData 最近一次缓存的测量
      @n 须先调用 update()。用 is_new 判断是否新样本，用 valid 判断预热是否完成。
    '''

  def get_single_gas_data(self, data_type)
    '''!
      @brief 返回缓存中的单个气体值
      @param data_type DATA_IAQ / DATA_TVOC / DATA_ETOH / DATA_ECO2 / DATA_RELIAQ
      @return 缓存值；当前模式不支持时为 NAN
    '''

  def get_sht40_data(self)
    '''!
      @brief 从模块缓存读取板载 SHT40 温湿度
      @return SHT40Data 温度（C）和湿度（%RH）
      @n 仅在 set_auto_humi_compensation(HUM_COMP_ENABLE) 之后固件才会读 SHT40。
    '''
```

## 兼容性

| Board        | Work Well | Work Wrong | Untested | Remarks |
| ------------ | :-------: | :--------: | :------: | ------- |
| RaspberryPi2 |           |            |    √     |         |
| RaspberryPi3 |           |            |    √     |         |
| RaspberryPi4 |           |            |    √     |         |

* Python Version

| Python  | Work Well | Work Wrong | Untested | Remarks |
| ------- | :-------: | :--------: | :------: | ------- |
| Python2 |           |            |    √     |         |
| Python3 |           |            |    √     |         |

## 历史

- 2026/09/12 - V1.0.0 版本

## 创作者

Written by PLELES(li.jia@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
