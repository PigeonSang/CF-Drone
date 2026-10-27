# MAVLink 库（header-only）包装为 IDF 组件。
#
# 草图里写的是 #include <MAVLink.h>，该头文件位于 Arduino 库根目录，
# 所以这里把库根目录整体加入 include path，无需复制文件。
#
# 若以后把库搬进仓库（推荐，便于版本锁定），把路径改成相对路径，例如：
#   set(MAVLINK_DIR "${CMAKE_CURRENT_LIST_DIR}/MAVLink")
#   idf_component_register(INCLUDE_DIRS "${MAVLINK_DIR}")

idf_component_register(
    INCLUDE_DIRS "C:/Users/Admin/Documents/Arduino/libraries/MAVLink"
)
