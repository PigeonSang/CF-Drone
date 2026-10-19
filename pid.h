// PID控制器实现
// PID controller implementation

#pragma once

#include "lpf.h"

class PID {
public:
	float p, i, d;
	float windup;
	float dtMax;

	float derivative = 0;
	float integral = 0;

	LowPassFilter<float> lpf; // 微分项低通滤波器

	PID(float p, float i, float d, float windup = 0, float dAlpha = 1, float dtMax = 0.1) :
		p(p), i(i), d(d), windup(windup), lpf(dAlpha), dtMax(dtMax) {}

	float update(float error) {
		float dt = t - prevTime;

		if (dt > 0 && dt < dtMax) {
			integral += error * dt;
			derivative = lpf.update((error - prevError) / dt); // 计算导数并应用低通滤波器
		} else {
			integral = 0;
			derivative = 0;
		}

		prevError = error;
		prevTime = t;

		return p * error + constrain(i * integral, -windup, windup) + d * derivative; // PID
	}

	void reset() {
		prevError = NAN;
		prevTime = NAN;
		integral = 0;
		derivative = 0;
		lpf.reset();
	}

private:
	float prevError = NAN;
	float prevTime = NAN;
};
