// Wi-Fi功能支持

#if WIFI_ENABLED

#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
#include "Preferences.h"

extern Preferences storage; // 复用主存储

const int W_DISABLED = 0, W_AP = 1, W_STA = 2;
int wifiMode = W_AP;
int udpLocalPort = 14550;
int udpRemotePort = 14550;
IPAddress udpRemoteIP = "255.255.255.255";

WiFiUDP udp;

void setupWiFi() {
	print("Setup Wi-Fi\n");
	WiFi.setSleep(false); // 禁用省电模式 - 必须在开启软AP/启动前设置
	if (wifiMode == W_AP) {
		WiFi.softAP(
			storage.getString("WIFI_AP_SSID", "Ciallo～(∠・ω< )⌒★").c_str(),
			storage.getString("WIFI_AP_PASS", "12345678").c_str()
		);
		// iPhone 的 802.11 省电和“低电量模式”无关，系统里关不掉。
		// 11n 聚合帧重传时，同一加密序号会再进一次协议栈，于是报 CCMP replay。
		// 遥控数据很小，热点只留 11b/g。
		esp_wifi_set_ps(WIFI_PS_NONE);
		esp_err_t wifiErr = esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G);
		if (wifiErr != ESP_OK) print("WiFi 关闭 11n 失败: %d\n", (int)wifiErr);
	} else if (wifiMode == W_STA) {
		WiFi.begin(
			storage.getString("WIFI_STA_SSID", "").c_str(),
			storage.getString("WIFI_STA_PASS", "").c_str()
		);
	}
	udp.begin(udpLocalPort);
}

void sendWiFi(const uint8_t *buf, int len) {
	if (WiFi.softAPgetStationNum() == 0 && !WiFi.isConnected()) return;

	// 必须"真的收到过"地面站的包才发。
	// 注意：不能只用 now - mavlinkLastRxMs > 5000 判断——上电后
	// mavlinkLastRxMs 初值是 0，前 5 秒会被误判成"刚收到过"，于是继续往
	// 广播地址 255.255.255.255 空发，pbuf 耗尽后 endPacket() 报 ENOMEM(12)，
	// lwIP 内存紧张会连带 HTTP 请求排队，表现为"开机后遥控延迟上千毫秒"。
	extern unsigned long mavlinkLastRxMs;
	extern bool mavlinkConnected;
	unsigned long now = millis();
	if (!mavlinkConnected) return;
	if (now - mavlinkLastRxMs > 5000) return;

	// 发失败后退避 1 秒，避免连续失败把 lwIP 内存打满
	static unsigned long blockedUntil = 0;
	if (now < blockedUntil) return;

	udp.beginPacket(udpRemoteIP, udpRemotePort);
	udp.write(buf, len);
	if (!udp.endPacket()) blockedUntil = now + 1000;
}

int receiveWiFi(uint8_t *buf, int len) {
	udp.parsePacket();
	if (udp.remoteIP()) udpRemoteIP = udp.remoteIP();
	return udp.read(buf, len);
}

void printWiFiInfo() {
	if (WiFi.getMode() == WIFI_MODE_AP) {
		print("Mode: Access Point (AP)\n");
		print("MAC: %s\n", WiFi.softAPmacAddress().c_str());
		print("SSID: %s\n", WiFi.softAPSSID().c_str());
		print("Password: ***\n");
		print("Clients: %d\n", WiFi.softAPgetStationNum());
		print("IP: %s\n", WiFi.softAPIP().toString().c_str());
	} else if (WiFi.getMode() == WIFI_MODE_STA) {
		print("Mode: Client (STA)\n");
		print("Connected: %d\n", WiFi.isConnected());
		print("MAC: %s\n", WiFi.macAddress().c_str());
		print("SSID: %s\n", WiFi.SSID().c_str());
		print("Password: ***\n");
		print("IP: %s\n", WiFi.localIP().toString().c_str());
	} else {
		print("Mode: Disabled\n");
		return;
	}
	print("Remote IP: %s\n", udpRemoteIP.toString().c_str());
	print("MAVLink connected: %d\n", mavlinkConnected);
}

void configWiFi(bool ap, const char *ssid, const char *password) {
	if (ap) {
		storage.putString("WIFI_AP_SSID", ssid);
		storage.putString("WIFI_AP_PASS", password);
	} else {
		storage.putString("WIFI_STA_SSID", ssid);
		storage.putString("WIFI_STA_PASS", password);
	}
	print("✓ 重启后生效 Reboot to apply new settings\n");
}

#endif
