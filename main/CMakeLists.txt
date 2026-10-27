# 把 Arduino 草图（16 个 .ino 合并为单一编译单元）与 drivers.cpp 纳入 IDF 构建。
#
# - main.cpp 按 arduino-cli 的合并顺序 #include 全部 .ino（见 build/.tmp-esp32s3/
#   sketch/CF-Drone.ino.cpp.merged 的 #line 标记），因此与现有 arduino-cli 编译行为等价。
# - drivers.cpp 内部已 #include 三个驱动实现，故这里只列 drivers.cpp，避免重复编译。
# - INCLUDE_DIRS 覆盖工程根目录（board_config.h / vector.h / util.h / web_rc_*.h 等）
#   与 drivers/（IMU.h / SBUS.h / MPU9250.h）。

idf_component_register(
    SRCS "main.cpp" "../drivers.cpp"
    INCLUDE_DIRS "." ".." "../drivers"
    # nvs_flash：parameters.ino 直接 #include <nvs_flash.h> 并调用
    # nvs_flash_init()/nvs_flash_erase()。arduino 组件只把 nvs_flash 列为
    # priv_reqs，不会传递给我们，必须显式声明。
    REQUIRES arduino mavlink nvs_flash
)

# 这套 Arduino 草图是按 Arduino IDE 的告警级别编写的，在 ESP-IDF 的 -Werror 下
# 会被大量风格类告警阻塞。注意：ESP-IDF 传的是 -Werror=all 以及一批 -Wno-error=xxx，
# 而“具体的 -Werror=foo”不会被笼统的 -Wno-error 覆盖，必须逐项关掉。
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error
    -Wno-error=comment                    # web_rc.ino 注释行尾的续行反斜杠
    -Wno-error=reorder                    # pid.h 成员初始化顺序
    -Wno-error=maybe-uninitialized
    -Wno-error=overloaded-virtual
    -Wno-error=address-of-packed-member
    -Wno-error=missing-field-initializers
)
