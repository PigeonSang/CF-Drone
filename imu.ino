// 陀螺仪加速度计相关设置
// Work with the IMU sensor

#include <SPI.h>
#include "drivers/MPU9250.h"
#include "vector.h"
#include "lpf.h"
#include "util.h"
#include "board_config.h"

MPU9250 imu(SPI, BOARD_SPI_CS);

bool imuOK = false; // IMU 初始化是否成功；false 时禁止解锁，readIMU() 跳过等待

// IMU 安装方向（欧拉角，单位 rad）。默认值 (0, 0, -PI/2) 对应本 PCB 的安装方式：
// 芯片正面朝上，X 丝印→飞行器右侧，Y 丝印→飞行器前方 → 转换公式 Vector(data.y, -data.x, data.z)
// 如需适配其他安装方向，修改此值并执行 `preset` 重置参数存储。
Vector imuRotation(0, 0, -PI / 2);

Vector accBias;
Vector accScale(1, 1, 1);
Vector gyroBias;
LowPassFilter<Vector> gyroBiasFilter(0.001); // 陀螺仪偏置低通估计滤波器

void setupIMU() {
	print("Setup IMU\n");
	SPI.begin(BOARD_SPI_SCK, BOARD_SPI_MISO, BOARD_SPI_MOSI, BOARD_SPI_CS);
	if (!imu.begin()) {
		print("⚠ IMU初始化失败！请检查 IMU 硬件连接！\n");
		return; // 不调用 configureIMU()，不启动定时器中断，imuOK 保持 false
	}
	imuOK = true;
	configureIMU();
}

void configureIMU() {
	imu.setAccelRange(imu.ACCEL_RANGE_4G);
	imu.setGyroRange(imu.GYRO_RANGE_2000DPS);
	imu.setDLPF(imu.DLPF_MAX);
	imu.setRate(imu.RATE_1KHZ_APPROX);
	imu.setupInterrupt();
}

void readIMU() {
	if (!imuOK) return; // IMU 故障时跳过，gyro/acc 保持零值，主循环继续运行
	imu.waitForData();
	imu.getGyro(gyro.x, gyro.y, gyro.z);
	imu.getAccel(acc.x, acc.y, acc.z);
	calibrateGyroOnce();
	// apply scale and bias
	acc = (acc - accBias) / accScale;
	gyro = gyro - gyroBias;
	// rotate to body frame using imuRotation Euler angles
	// 旋转四元数缓存：imuRotation 为运行时常量（仅参数变更时改变），
	// 缓存后每帧仅做 3 次浮点比较，消除原先每帧 6 次 sinf/cosf 调用。
	// 参数更新后首次 readIMU() 调用自动重建缓存，延迟仅 1 帧（~1 ms）。
	static Quaternion _imuRotQuat;
	static float _cachedRotX = NAN, _cachedRotY = NAN, _cachedRotZ = NAN;
	if (imuRotation.x != _cachedRotX || imuRotation.y != _cachedRotY || imuRotation.z != _cachedRotZ) {
		_imuRotQuat = Quaternion::fromEuler(imuRotation).inversed();
		_cachedRotX = imuRotation.x;
		_cachedRotY = imuRotation.y;
		_cachedRotZ = imuRotation.z;
	}
	acc  = Quaternion::rotateVector(acc,  _imuRotQuat);
	gyro = Quaternion::rotateVector(gyro, _imuRotQuat);
}

void calibrateGyroOnce() {
	static Delay landedDelay(2);
	if (!landedDelay.update(landed)) return; // 仅在设备完全静止时进行校准

	gyroBias = gyroBiasFilter.update(gyro);
}

void calibrateAccel() {
	if (armed) { print("请先上锁（disarm）后再校准\n"); return; }
	print("校准陀螺仪加速计 Calibrating accelerometer\n");
	imu.setAccelRange(imu.ACCEL_RANGE_2G); // 最灵敏模式

	print("1/6 水平放置：机头朝前（正常飞行方向），底部朝下水平放置在平坦表面，确保完全水平无倾斜。保持不动，8秒后开始校准；\n");
	pause(8);
	calibrateAccelOnce();
	print("水平校准完成。请继续。\n");
	pause(1);
	print("2/6 机头朝上：保持机头朝前,将机头指向天空，尾部朝下并接触支撑面，与地面垂直。保持不动，8秒后开始校准；\n");
	pause(8);
	calibrateAccelOnce();
	print("机头向上校准完成。请继续。\n");
	pause(1);
	print("3/6 机头朝下：保持机头朝前,将机头指向地面并接触支撑面，尾部朝上，与地面垂直。保持不动，8秒后开始校准；\n");
	pause(8);
	calibrateAccelOnce();
	print("机头向下校准完成。请继续。\n");
	pause(1);
	print("4/6 右侧朝下：保持机头朝前,将飞行器右侧朝下并接触支撑面，左侧机臂朝上，与地面垂直。保持不动，8秒后开始校准；\n");
	pause(8);
	calibrateAccelOnce();
	print("右侧向下校准完成。请继续。\n");
	pause(1);
	print("5/6 左侧朝下：保持机头朝前,将飞行器左侧朝下并接触支撑面，右侧机臂朝上，与地面垂直。保持不动，8秒后开始校准；\n");
	pause(8);
	calibrateAccelOnce();
	print("左侧向下校准完成。请继续。\n");
	pause(1);
	print("6/6 倒置放置：保持机头朝前,将飞行器完全翻转，顶部朝下、底部朝上，整体呈水平倒置状态。保持不动，8秒后开始校准；\n");
	pause(8);
	calibrateAccelOnce();

	printIMUCalibration();
	print("✓全部校准完成！将机身放正，执行ps命令查看校准结果。\n");
	configureIMU();
}

void calibrateAccelOnce() {
	const int samples = 1000;
	static Vector accMax(-INFINITY, -INFINITY, -INFINITY);
	static Vector accMin(INFINITY, INFINITY, INFINITY);

	// Compute the average of the accelerometer readings
	acc = Vector(0, 0, 0);
	for (int i = 0; i < samples; i++) {
		imu.waitForData();
		Vector sample;
		imu.getAccel(sample.x, sample.y, sample.z);
		acc = acc + sample;

	#ifdef CONFIG_IDF_TARGET_ESP32C3
		// 每10帧主动让出1ms，防止单核ESP32-C3上WiFi任务因连续采样而饿死导致Beacon丢失
		if (i % 10 == 0) delay(1);
	#endif
	
	}
	acc = acc / samples;

	// 更新最大值与最小值
	if (acc.x > accMax.x) accMax.x = acc.x;
	if (acc.y > accMax.y) accMax.y = acc.y;
	if (acc.z > accMax.z) accMax.z = acc.z;
	if (acc.x < accMin.x) accMin.x = acc.x;
	if (acc.y < accMin.y) accMin.y = acc.y;
	if (acc.z < accMin.z) accMin.z = acc.z;
	// 计算缩放系数与偏置
	accScale = (accMax - accMin) / 2 / ONE_G;
	accBias = (accMax + accMin) / 2;
}

void printIMUCalibration() {
	print("gyro bias: %f %f %f\n", gyroBias.x, gyroBias.y, gyroBias.z);
	print("accel bias: %f %f %f\n", accBias.x, accBias.y, accBias.z);
	print("accel scale: %f %f %f\n", accScale.x, accScale.y, accScale.z);
}

void printIMUInfo() {
	imu.status() ? print("status: ERROR %d\n", imu.status()) : print("status: OK\n");
	print("model: %s\n", imu.getModel());
	print("who am I: 0x%02X\n", imu.whoAmI());
	print("rate: %.0f\n", loopRate);
	print("gyro: %f %f %f\n", gyro.x, gyro.y, gyro.z);
	print("acc: %f %f %f\n", acc.x, acc.y, acc.z);
	imu.waitForData();
	Vector rawGyro, rawAcc;
	imu.getGyro(rawGyro.x, rawGyro.y, rawGyro.z);
	imu.getAccel(rawAcc.x, rawAcc.y, rawAcc.z);
	print("raw gyro: %f %f %f\n", rawGyro.x, rawGyro.y, rawGyro.z);
	print("raw acc: %f %f %f\n", rawAcc.x, rawAcc.y, rawAcc.z);
}

// ==================== 加计标定（为"手动扶平"优化）====================
// 相对于原来的 ca：
//  1) 每面先等"陀螺静止 0.4s"再采样，采 600 点取均值。原版固定 8s/1000 点，
//     手一抖、慢慢滑动、温度漂移都会把误差写进参数。
//  2) 每面采完立刻校验"摆正偏差"（主分量与合矢量夹角）和抖动幅度，
//     超标自动要求重做，最多 3 次，避免坏数据被写入。
//  3) 不预设"哪个姿态对应哪个轴"，按实测主分量自动归类，安装旋转不会错配。
//  4) 结束打印每面残差，能直接看出哪一面没放好。
// 注意：读的是 imu.getAccel() 的原始值（未减 accBias、未除 accScale），
//       否则会用旧标定值去标定新标定，越标越偏。

#if WEB_RC_ENABLED
extern bool webConsoleEnabled;
// 校准期间强制把输出写进网页日志缓冲：即使调试面板没打开，
// 事后打开面板也能看到完整的校准过程（否则 print 只走串口）。
struct WebLogGuard {
	bool prev;
	WebLogGuard() : prev(webConsoleEnabled) { webConsoleEnabled = true; }
	~WebLogGuard() { webConsoleEnabled = prev; }
};
#endif

static const char* accelPoseNames[6] = {
	"1/6 平放：底部朝下、机头朝前，放在水平面上",
	"2/6 机头朝上：尾部着地、机身垂直（可轻靠墙面）",
	"3/6 机头朝下：机头着地、机身垂直",
	"4/6 右侧朝下：右侧着地、机身垂直",
	"5/6 左侧朝下：左侧着地、机身垂直",
	"6/6 完全倒扣：顶部朝下、底部朝上，水平"
};

// 校准期间维持系统运转：读 IMU、推进时间与姿态、保持 WiFi/MAVLink 存活
static void calPump() {
	readIMU();
	step();
	estimate();
#if WEB_RC_ENABLED
	readWebRC();
#endif
#if WIFI_ENABLED
	static unsigned long lastMav = 0;
	if (millis() - lastMav >= 100) {
		lastMav = millis();
		processMavlink();
	}
#endif
}

static float vectorComponent(const Vector& v, int i) {
	return i == 0 ? v.x : (i == 1 ? v.y : v.z);
}

// 等待指定秒数，期间保持系统运转；每 2 秒打印一次倒计时
static void waitSeconds(int sec) {
	unsigned long start = millis();
	int nextTick = sec;
	while (millis() - start < (unsigned long)sec * 1000UL) {
		calPump();
		int left = sec - (int)((millis() - start) / 1000UL);
		if (left < nextTick && left > 0 && left % 2 == 0) {
			nextTick = left;
			print("  %d…\n", left);
		}
	}
}

// 等静止 → 采样 → 返回均值与标准差
static bool sampleAccelPose(Vector& avg, Vector& sd, int samples = 600) {
	unsigned long t0 = millis();
	unsigned long stillSince = 0;
	bool still = false, ok = false;
	while (millis() - t0 < 6000) {
		calPump();
		if (gyro.norm() < 0.03f) {
			if (!still) { still = true; stillSince = millis(); }
			else if (millis() - stillSince > 400) { ok = true; break; }
		} else {
			still = false;
		}
	}
	if (!ok) return false;

	Vector sum, sum2;
	for (int i = 0; i < samples; i++) {
		calPump();
		Vector v;
		imu.getAccel(v.x, v.y, v.z); // 原始值，传感器系
		sum += v;
		sum2 += Vector(v.x * v.x, v.y * v.y, v.z * v.z);
		if ((i & 63) == 0) delay(1); // 让 WiFi 任务喘口气
	}
	avg = sum / (float)samples;
	Vector var = sum2 / (float)samples - Vector(avg.x * avg.x, avg.y * avg.y, avg.z * avg.z);
	sd = Vector(sqrtf(fmaxf(var.x, 0.0f)), sqrtf(fmaxf(var.y, 0.0f)), sqrtf(fmaxf(var.z, 0.0f)));
	return true;
}

bool calibrateAccelGuided() {
#if WEB_RC_ENABLED
	WebLogGuard webLogGuard;
#endif
	if (armed) { print("请先上锁（disarm）后再校准\n"); return false; }

	print("========== 加速度计六面校准（手动扶平版）==========\n");
	print("每面放稳后手离开，看到 ✓ 再换下一面；摆偏超过 12 度会自动要求重做。\n");

	imu.setAccelRange(imu.ACCEL_RANGE_2G);

	Vector m[6];
	int    axis[6];
	float  dir[6];
	int    got = 0;

	for (int pose = 0; pose < 6; pose++) {
		bool done = false;
		for (int attempt = 0; attempt < 3 && !done; attempt++) {
			print("%s%s\n", accelPoseNames[pose], attempt ? "（重试）" : "");
			print("  8 秒后开始采样，请把机身放稳…\n");
			waitSeconds(8);
			print("  采样中…\n");
			Vector avg, sd;
			if (!sampleAccelPose(avg, sd)) {
				print("   ✗ 6 秒内没检测到静止，请放稳再试\n");
				continue;
			}

			int a = 0;
			float best = fabsf(avg.x);
			if (fabsf(avg.y) > best) { a = 1; best = fabsf(avg.y); }
			if (fabsf(avg.z) > best) { a = 2; best = fabsf(avg.z); }

			float mag = avg.norm();
			if (mag < 1.0f) { print("   ✗ 读数异常，请检查 IMU\n"); continue; }

			float tilt = degrees(acosf(constrain(best / mag, 0.0f, 1.0f)));
			float s = (vectorComponent(avg, a) >= 0) ? 1.0f : -1.0f;

			print("   采样 (%+.3f, %+.3f, %+.3f)  |a|=%.3f  偏斜 %.1f 度  抖动 %.3f\n",
			      avg.x, avg.y, avg.z, mag, tilt, sd.norm());

			if (tilt > 12.0f) { print("   ✗ 摆得太斜（>12 度），请重放\n"); continue; }
			if (sd.norm() > 0.25f) { print("   ✗ 采样期间有抖动，请放稳\n"); continue; }

			bool dup = false;
			for (int k = 0; k < got; k++) {
				if (axis[k] == a && dir[k] == s) dup = true;
			}
			if (dup) { print("   ✗ 这个方向已经采过，请按提示换姿态\n"); continue; }

			m[got] = avg;
			axis[got] = a;
			dir[got] = s;
			got++;
			print("   ✓ 已记录 %d/6\n", got);
			done = true;
		}
		if (!done) {
			print("✗ 这一面连续 3 次失败，校准取消（参数未改变）\n");
			configureIMU();
			return false;
		}
	}

	Vector pos[3], neg[3];
	bool hasPos[3] = {false, false, false}, hasNeg[3] = {false, false, false};
	for (int k = 0; k < 6; k++) {
		if (dir[k] > 0) { pos[axis[k]] = m[k]; hasPos[axis[k]] = true; }
		else            { neg[axis[k]] = m[k]; hasNeg[axis[k]] = true; }
	}
	for (int i = 0; i < 3; i++) {
		if (!hasPos[i] || !hasNeg[i]) {
			print("✗ %c 轴缺少一个方向（缺 %s），校准取消\n",
			      (char)('X' + i), hasPos[i] ? "负向" : "正向");
			configureIMU();
			return false;
		}
	}

	Vector newBias, newScale;
	for (int i = 0; i < 3; i++) {
		float vp = vectorComponent(pos[i], i);
		float vn = vectorComponent(neg[i], i);
		float b  = (vp + vn) * 0.5f;
		float sc = (vp - vn) * 0.5f / ONE_G;
		if (i == 0) { newBias.x = b; newScale.x = sc; }
		if (i == 1) { newBias.y = b; newScale.y = sc; }
		if (i == 2) { newBias.z = b; newScale.z = sc; }
	}

	for (int i = 0; i < 3; i++) {
		float sc = vectorComponent(newScale, i);
		if (sc < 0.85f || sc > 1.15f) {
			print("✗ %c 轴刻度解算为 %.3f（应在 0.85~1.15），多半有面没放好，校准取消\n",
			      (char)('X' + i), sc);
			configureIMU();
			return false;
		}
	}

	print("---- 每面残差（m/s^2，越小越好；>0.3 说明该面没放好）----\n");
	float maxErr = 0.0f;
	for (int k = 0; k < 6; k++) {
		float g = dir[k] * ONE_G;
		Vector pred;
		pred.x = (axis[k] == 0 ? newScale.x * g : 0.0f) + newBias.x;
		pred.y = (axis[k] == 1 ? newScale.y * g : 0.0f) + newBias.y;
		pred.z = (axis[k] == 2 ? newScale.z * g : 0.0f) + newBias.z;
		float err = (pred - m[k]).norm();
		if (err > maxErr) maxErr = err;
		print("  第 %d 面  %.3f\n", k + 1, err);
	}

	accBias  = newBias;
	accScale = newScale;
	configureIMU();

	printIMUCalibration();
	print("✓ 校准完成，最大残差 %.3f。放平后执行 ps 检查零点。\n", maxErr);
	return true;
}

// 网页「自动校正」：六面成功后接着做水平零点。最后一面是倒扣，
// 先等机身被翻动，再进入 cl，避免把倒扣姿态写成配平。
void calibrateAccelThenLevel() {
	if (!calibrateAccelGuided()) return;
	print("六面完成。请把飞机翻回水平面，手离开…\n");
	unsigned long t0 = millis();
	bool moved = false;
	while (millis() - t0 < 30000) {
		calPump();
		if (gyro.norm() > 0.15f) { moved = true; break; }
	}
	if (!moved) {
		print("✗ 30 秒内没有检测到翻转机身，已跳过水平校正。放平后可单独执行 cl。\n");
		return;
	}
	calibrateLevel();
}

// ==================== 水平零点校正（自动写配平）====================
// 前提：放置面必须真的水平。它把"当前估计的姿态角"写进 CTL_TRIM_*，
// 让控制器以该值为目标——如果台面本身是斜的，会把斜度当成水平，反而引入漂移。
void calibrateLevel() {
#if WEB_RC_ENABLED
	WebLogGuard webLogGuard;
#endif
	if (armed) { print("请先上锁后再做水平校正\n"); return; }

	print("========== 水平零点校正 ==========\n");
	print("前提：放置面必须水平（建议先用手机水平仪确认）。\n");
	print("把飞机放平，手离开，保持不动…\n");

	unsigned long t0 = millis();
	unsigned long stillSince = 0;
	bool still = false, ok = false;
	while (millis() - t0 < 6000) {
		calPump();
		if (gyro.norm() < 0.03f) {
			if (!still) { still = true; stillSince = millis(); }
			else if (millis() - stillSince > 800) { ok = true; break; }
		} else {
			still = false;
		}
	}
	if (!ok) { print("✗ 没有检测到静止\n"); return; }

	Vector sum;
	const int N = 800;
	for (int i = 0; i < N; i++) {
		calPump();
		sum += attitude.toEuler();
		if ((i & 63) == 0) delay(1);
	}
	Vector e = sum / (float)N;

	print("实测姿态：roll %+.2f 度   pitch %+.2f 度\n", degrees(e.x), degrees(e.y));
	setParameter("CTL_TRIM_ROLL",  e.x);
	setParameter("CTL_TRIM_PITCH", e.y);
	print("已写入 CTL_TRIM_ROLL=%.4f  CTL_TRIM_PITCH=%.4f（弧度）\n", e.x, e.y);
	print("✓ 完成。ps 读数不会变，需试飞验证漂移方向。\n");
}
