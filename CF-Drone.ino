// 主程序
// 使用Arduino IDE 2.3.x以上版本编译，不支持老版本的1.8.19！
// 必须安装ESP32开发板核心库（即esp32 by Espressif Systems）和依赖（MAVLINK等）

#include "vector.h"
#include "quaternion.h"
#include "util.h"
#include "board_config.h"
#include "esp_log.h"

// WiFi 和 Web 遥控器开关由 board_config.h 按芯片自动设置：
// 如需手动覆盖，在此处 #undef 后重新 #define
#define WIFI_ENABLED    BOARD_WIFI_ENABLED
#define WEB_RC_ENABLED  BOARD_WEB_RC_ENABLED

float t = NAN; // 当前步进时间，单位：秒
float dt; // 与上一步进的时间差，单位：秒
float controlRoll, controlPitch, controlYaw, controlThrottle; // 飞手输入指令，范围 [-1, 1]
float controlMode = NAN;
Vector gyro; // 陀螺仪数据
Vector acc; // 加速度计数据，单位：m/s/s
Vector rates; // 滤波后的角速度，单位：rad/s
Quaternion attitude; // 估计出的姿态（四元数）
bool landed; // are we landed and stationary

void setup() {
	// 强制满频 240MHz。WiFi 省电/DFS 或异常复位后 CPU 可能停在 80/160MHz，
	// 那样整个控制循环的开销会变成 2~3 倍（实测 285→371Hz 的残余很可能就是它）。
	// 必须在 Serial.begin 之前调用，否则 UART 分频不会跟着更新，串口会乱码。
	setCpuFrequencyMhz(240);
	Serial.begin(115200); // 初始化串口，波特率115200
	// 关闭 ESP-IDF 的 INFO 日志。WiFi 栈在 115200 波特率下每行日志约 7~9ms，
	// 会在串口 TX 上排队并阻塞主循环，实测造成 17~20ms 的 dt 尖峰。
	// 只保留 WARN/ERROR，WiFi 真出问题时仍然能看到。
	esp_log_level_set("*", ESP_LOG_WARN);
	disableBrownOut(); // 禁用ESP32低压复位检测，防止电机启动瞬间电压跌落导致误复位
	print("琛光科技 CF-Drone Flight Controller\n"); // 品牌标识（LICENSE 附加条款第二条要求固件启动输出保留品牌名称）
	print("程序开始初始化！\n");
	setupParameters(); // 从Flash加载参数（未存储时使用默认值）
	setupLED(); // 初始化状态指示灯
	setupMotors(); // 初始化电机输出（PWM/DShot）
	setLED(true); // 点亮LED，提示正在初始化
#if WIFI_ENABLED
	setupWiFi(); // 初始化WiFi（用于Web遥控/MAVLink等）
#endif
#if WEB_RC_ENABLED
	setupWebRC();  // 初始化Web遥控器
#endif
	setupIMU(); // 初始化IMU（陀螺仪/加速度计）
	setupRC(); // 初始化遥控接收机（SBUS/ELRS等协议）
	setLED(false); // 熄灭LED，提示初始化完成
	print("程序初始化完成！\n");
	print("================================\n");
}

// ===== 分段计时（开销约 8us/圈，可忽略）=====
// 用 CLI 命令 `prof` 打印各段平均耗时并清零，用来定位主循环变慢的原因。
uint32_t profUs[8];
uint32_t profN;

void loop() {
	uint32_t a;

	a = micros(); readIMU();   profUs[0] += micros() - a;
	a = micros(); step();      profUs[1] += micros() - a;
	a = micros(); readRC();    profUs[2] += micros() - a;
#if WEB_RC_ENABLED
	a = micros(); readWebRC(); processConsoleCommandQueue(); profUs[3] += micros() - a;
#endif
	a = micros(); estimate();  profUs[4] += micros() - a;
	a = micros();
	updateBatteryVoltage();
	control();
	updateMotorTest();
	sendMotors();
	profUs[5] += micros() - a;
	a = micros();
	handleInput();
#if WIFI_ENABLED
	processMavlink();
#endif
	profUs[6] += micros() - a;
	a = micros();
	logData();
	syncParameters();
	updateLED();
	profUs[7] += micros() - a;

	profN++;
}
