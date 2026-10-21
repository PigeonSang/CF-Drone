// 使用MOSFET的电机输出控制
// 如果使用ESC，将pwmStop、pwmMin和pwmMax更改为适当的μs值，将pwmFrequency减小到400
// Motors output control using MOSFETs
// In case of using ESCs, change pwmStop, pwmMin and pwmMax to appropriate values in μs, decrease pwmFrequency (to 400)

#include "util.h"
#include "board_config.h"

float motors[4]; // normalized motor thrusts in range [0..1]

// 电机引脚（对应 MOTOR_REAR_LEFT=0, MOTOR_REAR_RIGHT=1, MOTOR_FRONT_RIGHT=2, MOTOR_FRONT_LEFT=3）
int motorPins[4] = BOARD_MOTOR_PINS; // RL, RR, FR, FL

int pwmFrequency = 25000; // ESP32-C3 LEDC 默认 XTAL 时钟 40MHz，10-bit 上限 39062Hz；
                          // ESP32 APB 80MHz，10-bit 上限 78125Hz；
                          // 25kHz 在两者 XTAL/APB 下 prescaler 均可精确表示，安全余量充足
int pwmResolution = 10;
int pwmStop = 0;
int pwmMin = 0;
int pwmMax = -1; // -1 表示纯占空比模式（接 MOSFET 直驱）；接 ESC 时设为实际 PWM 最大值（μs）

// Motors array indexes:
const int MOTOR_REAR_LEFT = 0;
const int MOTOR_REAR_RIGHT = 1;
const int MOTOR_FRONT_RIGHT = 2;
const int MOTOR_FRONT_LEFT = 3;

void setupMotors() {
	print("Setup Motors\n");

	// 先解绑所有引脚（重复调用时清理旧 LEDC 通道），再拉低防止误转
	for (int i = 0; i < 4; i++) {
		ledcDetach(motorPins[i]);         // 首次调用时无绑定会静默返回 false，无副作用
		pinMode(motorPins[i], OUTPUT);
		digitalWrite(motorPins[i], LOW);
	}

	// configure pins
	for (int i = 0; i < 4; i++) {
		bool ok = ledcAttach(motorPins[i], pwmFrequency, pwmResolution);
		if (ok) {
			double actual = ledcChangeFrequency(motorPins[i], pwmFrequency, pwmResolution);
			if (actual > 0) pwmFrequency = (int)round(actual); // 用 double 接收返回值，避免精度损失
		}
		print("  motor%d pin=%d ledcAttach=%s\n", i, motorPins[i], ok ? "OK" : "FAIL");
	}

	sendMotors();
	print("Motors initialized\n");
}

int getDutyCycle(float value) {
	value = constrain(value, 0, 1);
	if (pwmMax >= 0) { // pwm 时间模式（接 ESC 用）
		float pwm = mapf(value, 0, 1, pwmMin, pwmMax);
		if (value == 0) pwm = pwmStop;
		float duty = mapf(pwm, 0, 1000000 / pwmFrequency, 0, (1 << pwmResolution) - 1);
		return round(duty);
	} else { // 纯占空比模式（接 MOSFET 直驱用）
		return round(value * ((1 << pwmResolution) - 1));
	}
}

void sendMotors() {
	for (int i = 0; i < 4; i++) {
		ledcWrite(motorPins[i], getDutyCycle(motors[i]));
	}
}

bool motorsActive() {
	return motors[0] != 0 || motors[1] != 0 || motors[2] != 0 || motors[3] != 0;
}

void testMotor(int n) {
	print("Testing motor %d\n", n);
	motors[n] = 0.3;
	delay(50); // ESP32可能需要等待当前周期结束才能更改占空比 https://github.com/espressif/arduino-esp32/issues/5306
	sendMotors();
	pause(3);
	motors[n] = 0;
	sendMotors();
	print("Done\n");
}

// ===== 马达自测：四路依次转动，每路 2 秒，无视上锁状态（仅限台架，务必拆桨）=====
#define SELF_TEST_SPIN_TIME 2.0f
#define SELF_TEST_GAP_TIME  0.4f
#define SELF_TEST_DUTY      0.3f

static const int SELF_TEST_ORDER[4] = {MOTOR_FRONT_LEFT, MOTOR_FRONT_RIGHT, MOTOR_REAR_LEFT, MOTOR_REAR_RIGHT};
int   selfTestIndex  = -1;    // -1 = 空闲；0~3 = 当前第几个电机
float selfTestNextAt = 0;     // 下一阶段切换时间
bool  selfTestSpin   = false; // 当前阶段是否在转动

void startMotorSelfTest() {
	selfTestIndex  = 0;
	selfTestSpin   = false;
	selfTestNextAt = t + SELF_TEST_GAP_TIME;
	print("Motor self-test: FL -> FR -> RL -> RR\n");
}

void stopMotorSelfTest() {
	selfTestIndex = -1;
	selfTestSpin  = false;
}

void updateMotorTest() {
	if (selfTestIndex < 0) return; // 空闲：不干预 control() 的输出

	if (t >= selfTestNextAt) {
		if (!selfTestSpin) {          // 间隔结束 -> 开始转
			selfTestSpin   = true;
			selfTestNextAt = t + SELF_TEST_SPIN_TIME;
		} else {                      // 转动结束 -> 进入间隔/下一个
			selfTestSpin   = false;
			selfTestNextAt = t + SELF_TEST_GAP_TIME;
			if (++selfTestIndex >= 4) selfTestIndex = -1;
		}
	}

	for (int i = 0; i < 4; i++) motors[i] = 0.0f;
	if (selfTestIndex >= 0 && selfTestSpin)
		motors[SELF_TEST_ORDER[selfTestIndex]] = SELF_TEST_DUTY;
}
