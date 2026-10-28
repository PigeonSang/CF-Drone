// 时间相关函数
// Time related functions

float loopRate; // Hz

void step() {
	// micros() 是 32 位，约 71.6 分钟回绕一次。
	// 旧实现直接把 micros() 当时间基准，回绕瞬间 t 会从 4294 跳回 0，
	// 所有保存了绝对时刻的逻辑都会失效：Rate 限速器（遥测/日志/参数落盘停摆）、
	// Delay（陀螺零偏学习失效）、电池采样、马达自测状态机卡死。
	// 改成"无符号差值算 dt + t 单调累加"，t 永不再回绕。
	static uint32_t lastUs = 0;
	static bool     started = false;
	uint32_t nowUs = micros();

	if (!started) {
		lastUs = nowUs;
		started = true;
		t = 0.0f; // 全局 t 初值是 NAN；累加模式必须先归零
	}

	float d = (nowUs - lastUs) * 1e-6f;
	lastUs = nowUs;

	// 过滤异常跳变：首圈、复位、长时间阻塞（>0.5s）都不参与积分
	if (d > 0.0f && d < 0.5f) {
		dt = d;
	} else {
		dt = 0.0f;
	}

	t += dt;
	computeLoopRate();
}

void computeLoopRate() {
	static float windowStart = 0;
	static uint32_t rate = 0;
	rate++;
	if (t - windowStart >= 1) { // 1秒窗口
		loopRate = rate;
		windowStart = t;
		rate = 0;
	}
}
