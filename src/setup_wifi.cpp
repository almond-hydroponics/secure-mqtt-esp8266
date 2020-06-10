#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFiMulti.h>
#include <ctime>
#include "globals.h"
#include "setup_wifi.h"
#include "config.h"

ESP8266WiFiMulti wifiMulti;

// Static IP configuration
#define IPSET_STATIC { 10, 10, 10, 13 }
#define IPSET_GATEWAY { 10, 10, 10, 254 }
#define IPSET_SUBNET { 255, 255, 255, 0 }
#define IPSET_DNS { 10, 10, 10, 254 }

byte ip_static[] = IPSET_STATIC;
byte ip_gateway[] = IPSET_GATEWAY;
byte ip_subnet[] = IPSET_SUBNET;
byte ip_dns[] = IPSET_DNS;

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;

// Set time via NTP, as required for x.509 validation
void SetupWifi::setClock()
{
	//void configTime(int timezone, int daylightOffset_sec, const char* server1, const char* server2, const char* server3)
	// https://github.com/esp8266/Arduino/blob/master/cores/esp8266/time.cpp
	configTime(3600,
			   0,
			   "pool.ntp.org",
			   "time.nist.gov",
			   "time.windows.com"
			   );
//	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1);
	setClock_status = STARTED;
	DEBUG_LOG("Waiting for NTP time sync: ");
	setClock_AsyncWait.startWaiting(millis(), 1000);    // Log every 1 second
	// Asynchronously wait for network response via checkClockStatus().
}

// Check Clock Status and update 'setClock_status' accordingly.
void SetupWifi::checkClockStatus()
{
	time_t now = time(nullptr);
//	while (now < 8 * 3600 * 2) {
//		delay(500);
//		DEBUG_LOG(".");
//		now = time(nullptr);
//	}
	if (now < 8 * 3600 * 2) {
		// The NTP request has not yet completed.
		if (!setClock_AsyncWait.isWaiting(millis())) {
			DEBUG_LOG(".");
			setClock_AsyncWait.startWaiting(millis(), 1000); // Log every 1 second.
		}
		return;
	}

	// The NTP request has completed
	setClock_status = SUCCESS;

#ifdef DEBUG
		struct tm timeinfo{};
		gmtime_r(&now, &timeinfo);
		DEBUG_LOGLN("");
		DEBUG_LOG("Current time: ");
		DEBUG_LOGLN(asctime(&timeinfo));
#endif
}

String SetupWifi::getMacAddress()
{
	byte mac[6];
	String macStr;

	WiFi.macAddress(mac);
	macStr = String(mac[0], HEX) + ":"
		+ String(mac[1], HEX) + ":"
		+ String(mac[2], HEX) + ":"
		+ String(mac[3], HEX) + ":"
		+ String(mac[4], HEX) + ":"
		+ String(mac[5], HEX);

	return macStr;
}

void wifiOnConnect()
{
    digitalWrite(PIN_LED, HIGH);
    delay(1000);
    digitalWrite(PIN_LED, LOW);
    delay(1000);
}

// Connect to WiFi Network.
void SetupWifi::setupWifi()
{
	if (WiFi.status() != WL_CONNECTED) {
		DEBUG_LOGLN("");
		DEBUG_LOG("MAC ");
		DEBUG_LOGLN(getMacAddress());
		DEBUG_LOG("Connecting to WiFi...");
		DEBUG_LOGLN(ssid);

		WiFi.mode(WIFI_STA);
		wifiMulti.addAP(ssid, password);
		WiFi.config(IPAddress(ip_static),
					IPAddress(ip_gateway),
					IPAddress(ip_subnet),
					IPAddress(ip_dns));

		int attempt = 0;
		while (wifiMulti.run() != WL_CONNECTED) {
			DEBUG_LOGLN("Connection Failed! Rebooting...");
			wifiOnConnect();
			DEBUG_LOG(".");
			DEBUG_LOG(attempt);
			delay(500);
			attempt++;
//			delay(5000);
//			ESP.restart();

			if (attempt == 150) {
				DEBUG_LOGLN("");
				DEBUG_LOGLN("Could not connect to WiFi");

				ESP.restart();
				delay(500);
			}
		}

		DEBUG_LOGLN("");
		DEBUG_LOGLN("WiFi connected");
		DEBUG_LOG("IP address: ");
		DEBUG_LOG(WiFi.localIP());
		//DEBUG_LOG(", MAC ");
		//DEBUG_LOG(getMacAddress());
		DEBUG_LOGLN("");
		randomSeed(micros());
		setClock();

		DEBUG_LOGLN("Ready.");
		digitalWrite(PIN_LED, HIGH);
		//DEBUG_LOG("IP address: ");
		//DEBUG_LOGLN(WiFi.localIP());
	}
}

// Loop WiFi
void SetupWifi::loopWifi()
{
	// Prevent ALL other actions here until the clock as been set by NTP.
	if (setClock_status < FINISHED) {
		checkClockStatus();
		return;
	}
	//MilliSec currentMilliSec = millis();
}
