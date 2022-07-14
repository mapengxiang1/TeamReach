<h1 align="center">使用指南</h1>

#### 目录

- [项目简介](#项目简介)

- [运行方式](#运行方式)

- [todolist](#todolist)

- [备注](#备注)

***

## 项目描述

<p>&emsp;&emsp;此程序项目为氧化锌电阻分拣系统主控器程序源代码，主要实现的功能包括分拣工件，显示输出，日志文件读写，数据实时上传等。</p>

***

## 运行方式

<p>&emsp;&emsp;在控制器上运行，需配合NBIoT通讯模块、机械臂、传感器等外设实现功能</p>

### 1.硬件配置

|名称|型号|
|---|---|
|主控芯片|龙芯1B200|
|主控板|龙芯LS1B开发板|
|显示屏|7寸LCD电阻屏 |
||分辨率800*480|
||主控芯片XPT2046|
|NBIoT模块|SIM7020C|
|机械臂|BRTIRUS1510A|
|压力传感器|ZEMIC LOAD CELL L60|

### 2.主控板接口定义

|接口定义|编号|插座:管脚|接线说明|
|---|---|---|---|
|ADC|ADC3|J10:2|接压力传感器输出管脚|
|INPUT_SIGNAL|GPIO34|P2:5|接机械臂输出脉冲管脚|
|INPUT_SIGNAL|GPIO37|P2:4|接机械臂输出脉冲管脚|
|OUTPUT_PULSE|GPIO36|P2:3|接机械臂输入脉冲管脚|
|UART|UART3|J3|经TTL电平转换模块接NBIoT模块|

### 3.软件配置

|软件名称|版本|安装说明|
|---|---|---|
| LoongIDE | 1.1及以上 | [安装方式](#http://www.loongide.com/upload/file/pdf/readme_first_time.pdf) |

***

## todolist

1. 扩展功能实现控制4路传感器与机械臂生产线；
2. 应用[LVGL图形库](#https://github.com/lvgl/lvgl)重新设计UI；
3. 添加对WiFi通讯功能支持。

***

## 备注

<p>暂无</p>