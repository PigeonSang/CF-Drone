// 时间相关函数
// Time related functions

float loopRate; // Hz

void step() {
	float now = micros() / 1000000.0;
	dt = now - t;
	t = now;

	if (!(dt > 0)) {
		dt = 0; // 第一步以及复位时，假定dt为零
	}

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
