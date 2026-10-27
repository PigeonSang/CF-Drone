// CF-Drone — ESP-IDF 构建入口（Arduino-as-component）
//
// 作用：把 arduino-cli 原本自动合并的 16 个 .ino 复刻为单一编译单元。
// 顺序与 build/.tmp-esp32s3/sketch/CF-Drone.ino.cpp.merged 里的 #line 标记完全一致：
//   CF-Drone → battery → cli → control → estimate → imu → led → log
//            → mavlink → motors → parameters → rc → safety → time → web_rc → wifi
//
// 注意：
// - .ino 交给 C++ 预处理器完全合法，无需改名。
// - WIFI_ENABLED / WEB_RC_ENABLED 由 CF-Drone.ino 在包含 board_config.h 后定义，
//   因此后续 .ino 里的 #if WIFI_ENABLED 必须排在其后才生效（顺序不可随意调整）。
// - setup() / loop() 由 arduino 组件自带的 app_main 调用，本文件不要定义 app_main。

#include <Arduino.h>

// Arduino 会在合并 .ino 时自动插入函数原型；ESP-IDF 不做这一步，必须自己提供。
// 本文件内容取自 arduino-cli 生成的 build/.tmp-*/sketch/CF-Drone.ino.cpp 原型段。
#include "prototypes.h"

#include "CF-Drone.ino"
#include "battery.ino"
#include "cli.ino"
#include "control.ino"
#include "estimate.ino"
#include "imu.ino"
#include "led.ino"
#include "log.ino"
#include "mavlink.ino"
#include "motors.ino"
#include "parameters.ino"
#include "rc.ino"
#include "safety.ino"
#include "time.ino"
#include "web_rc.ino"
#include "wifi.ino"
