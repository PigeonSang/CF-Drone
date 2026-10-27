# CF-Drone — ESP-IDF 工程入口（Arduino-as-component 方案）
#
# 前置条件：
#   1. 已在 VS Code 中执行 “ESP-IDF: Add Arduino ESP32 as ESP-IDF Component”
#      （生成 components/arduino，版本需与 Arduino 核心 3.3.12 对齐）
#   2. idf.currentSetup 指向 C:/Espressif/frameworks/esp-idf-v5.5.6/
#
# 本文件与 main/、components/ 均为新增文件，未改动任何 .ino 源码，
# arduino-cli 的 build/ 产物保持原样（IDF 使用 build-idf/）。

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(CF-Drone)
