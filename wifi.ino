// Wi-Fi功能支持

#if WIFI_ENABLED

#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiUdp.h>
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

	// 只有最近 5 秒真的收到过 MAVLink 包才发。
	// 否则目标 IP 可能是失效的广播地址/旧地址：每次发送都要 ARP 重试并占住
	// lwIP 的 pbuf，最终 endPacket() 持续报 ENOMEM(12) 刷屏。
	extern unsigned long mavlinkLastRxMs;
	unsigned long now = millis();
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
