#define MAJOR_VER "00"
#define MINOR_VER "01"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include "async_wait.h"
#include "globals.h"
#include "setup_wifi.h"
#include "config.h"
#include "secure_credentials.h"

SetupWifi setupWifi(
	CONFIG.wlan.ssid,
	CONFIG.wlan.password,
	CA_CERT_PROG,
	CLIENT_CERT_PROG,
	CLIENT_KEY_PROG
);

const char* mqtt_server = CONFIG.mqtt.mqtt_server;

static PubSubClient pubSubClient(setupWifi.getWiFiClient());

// Handle incoming messages from broker.
void mqttCallback(char *topic, byte *payload, unsigned int payloadLength)
{
	String topicStr;
	String payloadStr;

	for (int i = 0; i < payloadLength; i++) {
		payloadStr += (char)payload[i];
	}

	DEBUG_LOGLN("");
    DEBUG_LOG("Message arrived - [");
    DEBUG_LOG(topicStr);
    DEBUG_LOG("] ");
    DEBUG_LOGLN(payloadStr);
}
