// 由 arduino-cli 自动生成的原型块（一次性同步）
// 来源：build/.tmp-esp32s3/sketch/CF-Drone.ino.cpp 第 28~216 行（已去掉 #line 指令）
//
// 为什么需要它：Arduino IDE / arduino-cli 在合并 16 个 .ino 时，会在函数体之前
// 插入所有函数的原型声明（它们由 ctags 扫描生成），所以 CF-Drone.ino 里可以直接
// 调用 cli.ino / motors.ino 中后面才定义的函数。ESP-IDF 的 CMake 不做这一步，
// 因此必须自己提供，否则会出现成片的 "'xxx' was not declared in this scope"。
//
// 维护：新增/删除/修改 .ino 中的函数签名后，需同步更新本文件；可对照
// arduino-cli 重新生成的 build/.tmp-*/sketch/CF-Drone.ino.cpp 原型段。
// 本文件被 main/main.cpp 包含一次，故 static 声明与定义同属一个 TU，合法。

#pragma once

void setup();
void loop();

void updateBatteryVoltage();
float readBatteryVoltage();

void print(const char* format, ...);
void pause(float duration);
void handleInput();

void control();
void interpretControls();
void controlAttitude();
void controlRates();
void controlTorque();
void desaturate(float& a, float& b, float& c, float& d);
const char * getModeName();
void interpretWebRC();

void estimate();
void applyGyro();
void applyAcc();
void applyLevel();

void setupIMU();
void configureIMU();
void readIMU();
void calibrateGyroOnce();
void calibrateAccel();
void calibrateAccelOnce();
void calibrateAccelGuided();
void calibrateLevel();
void printIMUCalibration();
void printIMUInfo();

void setupLED();
void setLED(bool on);
void blinkLED();
bool batteryAlertActive();
bool ledAlertActive();
void updateLED();

void prepareLogData();
void logData();
void printLogHeader();
void printLogData();

void processMavlink();
void sendMavlink();
void sendMessage(const void *msg);
void receiveMavlink();
void handleMavlink(const void *_msg);
void mavlinkPrint(const char* str);
void sendMavlinkPrint();

void setupMotors();
int getDutyCycle(float value);
void sendMotors();
bool motorsActive();
void testMotor(int n);
void updateMotorTest();

void setupParameters();
int parametersCount();
const char * getParameterName(int index);
float getParameter(int index);
float getParameter(const char *name);
bool setParameter(const char *name, const float value);
void syncParameters();
void printParameters();
void resetParameters();

static uint8_t crsf_crc8(const uint8_t *buf, int len);
static bool parseCRSFBuffer();
static void initDefaultRCCalibration();
void setupRC();
bool readRC();
void normalizeRC();
void calibrateRC();
void calibrateRCChannel(float *channel, uint16_t in[16], uint16_t out[16], const char *str);
void printRCCalibration();

void failsafe();
void rcLossFailsafe();
void descend();
void autoFailsafe();
void webRCLossFailsafe();
void invertedFailsafe();
void batteryFailsafe();

void step();
void computeLoopRate();

bool enqueueConsoleCmd(const char* cmd);
void processConsoleCommandQueue();
float applyDeadzone(float norm, float deadzone);
float processAxis(float raw, float& lastValid);
float processThrottle(float raw);
void setWebRCInput(float roll, float pitch, float yaw, float throttle, uint16_t buttons);
const char * findJsonValue(const char* json, const char* key);
bool handleJSONProtocol(String& body);
void setWebRCWarn(const char* msg);
void handleWebRCRequest();
bool isWebRCEnabled();
static void handleRedirect8080();
void setupWebRC();
void readWebRC();

void setupWiFi();
void sendWiFi(const uint8_t *buf, int len);
int receiveWiFi(uint8_t *buf, int len);
void printWiFiInfo();
void configWiFi(bool ap, const char *ssid, const char *password);
