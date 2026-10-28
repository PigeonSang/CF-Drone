// 只给 clangd 用。固件构建不包含这个文件。
// .ino 会被拼进 main.cpp 这一个编译单元；单独打开时前面的文件还不存在，
// 这里按 main/main.cpp 的顺序把“当前文件之前”的片段先包含进来。
// 当前文件自己由编译命令作为主文件解析，用 CLANGD_PART 跳过，避免重复定义。
#ifndef CLANGD_PART
#define CLANGD_PART 0
#endif

#include <Arduino.h>
#include "prototypes.h"

#if CLANGD_PART > 0
#include "CF-Drone.ino"
#endif
#if CLANGD_PART > 1
#include "battery.ino"
#endif
#if CLANGD_PART > 2
#include "cli.ino"
#endif
#if CLANGD_PART > 3
#include "control.ino"
#endif
#if CLANGD_PART > 4
#include "estimate.ino"
#endif
#if CLANGD_PART > 5
#include "imu.ino"
#endif
#if CLANGD_PART > 6
#include "led.ino"
#endif
#if CLANGD_PART > 7
#include "log.ino"
#endif
#if CLANGD_PART > 8
#include "mavlink.ino"
#endif
#if CLANGD_PART > 9
#include "motors.ino"
#endif
#if CLANGD_PART > 10
#include "parameters.ino"
#endif
#if CLANGD_PART > 11
#include "rc.ino"
#endif
#if CLANGD_PART > 12
#include "safety.ino"
#endif
#if CLANGD_PART > 13
#include "time.ino"
#endif
#if CLANGD_PART > 14
#include "web_rc.ino"
#endif
#if CLANGD_PART > 15
#include "wifi.ino"
#endif
