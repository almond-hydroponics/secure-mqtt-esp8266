#define MAJOR_VER "00"
#define MINOR_VER "01"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <EEPROM.h>

#include "async_wait.h"
#include "globals.h"
#include "setup_wifi.h"
#include "config.h"
#include "secure_credentials.h"

#ifndef STASSID
#define STASSID CONFIG.wlan.ssid
#define STAPSK  CONFIG.wlan.password
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

String relayPrefix = "RELAY";
String sensorPrefix = "SENSOR";

SetupWifi setupWifi(
	ssid,
	password,
	CA_CERT_PROG,
	CLIENT_CERT_PROG,
	CLIENT_KEY_PROG
);

const char* mqtt_server = CONFIG.mqtt.mqtt_server;

const char *ID = "sec_mqtt_client_" MAJOR_VER "_" MINOR_VER;	// Name of unique device
const String TOPIC_PUMP("almond/pump");

static PubSubClient pubSubClient(setupWifi.getWiFiClient());

// SPI setup.
const int slaveSelectPin = 16;
static SPISettings spiZoneSettings(2000000, MSBFIRST, SPI_MODE0);

// For Future Use.
const int programResetPin = 4;
const int spiRxRequestPin = 5;

void updateRelays(uint8_t bitmap) {
    // A low output value energizes the relay.
    // A high value turns off the relay.
    // So invert each bit.
    bitmap ^= 0xFF;

    DEBUG_LOG("updateRelays: ");
    DEBUG_LOGLN(String(bitmap, BIN));

    SPI.beginTransaction(spiZoneSettings);
    digitalWrite(slaveSelectPin, LOW);
    SPI.transfer(bitmap);
    digitalWrite(slaveSelectPin, HIGH);
    SPI.endTransaction();
}

// DATA FOR THIS DEVICE ONLY
int sensorPin5 = 5;
bool sensorPin5enabled = false;
int lastSensorPin5Value = 1;
int sensorPin6 = 6;
bool sensorPin6enabled = false;
int lastSensorPin6Value = 1;
int sensorPin7 = 7;
bool sensorPin7enabled = false;
int lastSensorPin7Value = 1;
int sensorPin8 = 8;
bool sensorPin8enabled = false;
int lastSensorPin8Value = 1;

int pinConverter(int boardPin)
{
    if (boardPin == 0) return 16;
    if (boardPin == 1) return 5;
    if (boardPin == 2) return 4;
    if (boardPin == 3) return 0;
    if (boardPin == 4) return 2;
    if (boardPin == 5) return 14;
    if (boardPin == 6) return 12;
    if (boardPin == 7) return 13;
    if (boardPin == 8) return 15;
}

// RESTORE SETTINGS TO PINS
void restoreSettingsToPins()
{
    for (int pinDex = 1; pinDex <= 4; pinDex++) {
        int pinValue = (int)EEPROM.read(pinDex);
        if (pinValue == 1 || pinValue == 0) {
            digitalWrite(pinConverter(pinDex),pinValue);
            Serial.println("Restoring " + String(pinDex) + ": " + String(pinValue));
        }
    }
    int pinValue = (int)EEPROM.read(5);
    sensorPin5enabled = pinValue == 1;
    pinValue = (int)EEPROM.read(6);
    sensorPin6enabled = pinValue == 1;
    pinValue = (int)EEPROM.read(7);
    sensorPin7enabled = pinValue == 1;
    pinValue = (int)EEPROM.read(8);
    sensorPin8enabled = pinValue == 1;
}

// RELAY METHODS
void processMsgForRelay(String relayMessage)
{
    relayMessage.replace(relayPrefix,"");

    String pinNoStr = relayMessage.substring(0,relayMessage.indexOf("="));
    String pinValueStr = relayMessage.substring(relayMessage.indexOf("=") + 1,relayMessage.length());

    int pinNo = pinConverter(pinNoStr.toInt());
    int pinValue = pinValueStr.toInt() <= 0 ? HIGH : LOW;
    Serial.println("RELAY PIN:" + String(pinNoStr.toInt()) + " VALUE:" + String(pinValue));
    EEPROM.write(pinNoStr.toInt(),pinValue);
    EEPROM.commit();
    digitalWrite(pinNo,pinValue);
}

// SENSOR METHODS
void processMsgForSensor(String sensorMessage)
{
    sensorMessage.replace(sensorPrefix,"");

    String sensorNoStr = sensorMessage.substring(0,sensorMessage.indexOf("="));
    String sensorStatusStr = sensorMessage.substring(sensorMessage.indexOf("=") + 1,sensorMessage.length());

    long sensorNo = sensorNoStr.toInt();
    bool sensorEnabled = sensorStatusStr.toInt() > 0;

    if (sensorNo == 5) sensorPin5enabled = sensorEnabled;
    if (sensorNo == 6) sensorPin6enabled = sensorEnabled;
    if (sensorNo == 7) sensorPin7enabled = sensorEnabled;
    if (sensorNo == 8) sensorPin8enabled = sensorEnabled;
    Serial.println("SENSOR PIN:" + String(sensorNo) + " VALUE:" + String(sensorEnabled));
    EEPROM.write(sensorNo,sensorEnabled);
    EEPROM.commit();
}

// MQTT METHODS
void processMQTTMessage(const String& message)
{
    DEBUG_LOG("MESSAGE: ");
    DEBUG_LOGLN(message);

    bool msgIdentified = false;
    // PROCESS RELAY PREFIXED MESSAGES
    if (message.indexOf(relayPrefix) >= 0) {
        processMsgForRelay(message);
    }
    // PROCESS SENSOR PREFIXED MESSAGES
    if (message.indexOf(sensorPrefix) >= 0) {
        processMsgForSensor(message);
    }
}

// Handle incoming messages from broker.
void mqttCallback(char *topic, byte *payload, unsigned int payloadLength)
{
	String topicStr;
	String payloadStr;

//	for (int i = 0; topic[i]; i++) {
//		topicStr += topic[i];
//	}

	for (int i = 0; i < payloadLength; i++) {
		payloadStr += (char)payload[i];
	}

	//TODO: In future to implement topic in array method
	topicStr = topic;

	DEBUG_LOGLN("");
    DEBUG_LOG("Message arrived - [");
    DEBUG_LOG(topicStr);
    DEBUG_LOG("] ");
    DEBUG_LOGLN(payloadStr);

    processMQTTMessage(payloadStr);

    //TODO: Remove or refactor once method is implemented
    if ((char)payload[0] == '1') {
        digitalWrite(PIN_PUMP, HIGH);
        DEBUG_LOG("PUMP ON");
    } else {
        digitalWrite(PIN_PUMP, LOW);
        DEBUG_LOG("PUMP OFF");
    }
}

void reconnectToMQTT(MilliSec currentMilliSec)
{
	const char* mqttUser = CONFIG.mqtt.mqtt_user;
    const char* mqttPassword = CONFIG.mqtt.mqtt_password;

	if (pubSubClient.connected()) return;

	static AsyncWait waitToRetry;
	if (waitToRetry.isWaiting(currentMilliSec)) return;

	DEBUG_LOG("Attempting MQTT connection: ");
	if (pubSubClient.connect(ID, mqttUser, mqttPassword)) {
		DEBUG_LOGLN("Connected");

		pubSubClient.subscribe(TOPIC_PUMP.c_str());
		DEBUG_LOG("Subscribed to: ");
		DEBUG_LOGLN(TOPIC_PUMP);
	} else {
		DEBUG_LOGLN("Try again in 5 seconds");
		waitToRetry.startWaiting(currentMilliSec, 5000);
	}
}

void loginToMQTT()
{
    const char* mqttServer = CONFIG.mqtt.mqtt_server;
    int mqttPort = CONFIG.mqtt.mqtt_port;
    const char* mqttUser = CONFIG.mqtt.mqtt_user;
    const char* mqttPassword = CONFIG.mqtt.mqtt_password;

    pubSubClient.setServer(mqttServer, mqttPort);
    pubSubClient.setCallback(mqttCallback);

//    DEBUG_LOG("Starting MQTT...");
//    while (!pubSubClient.connected()) {
//        DEBUG_LOG("Connecting to MQTT...: ");
//        DEBUG_LOGLN(pubSubClient.state());
//        if (pubSubClient.connect(ID, mqttUser, mqttPassword)) {
//            DEBUG_LOG("Connected as :");
//            DEBUG_LOGLN(String(ID));
//        }
//        else {
//            DEBUG_LOG("Failed with state: ");
//            DEBUG_LOGLN(pubSubClient.state());
//            delay(1000);
//        }
//    }
}

void setup()
{
	// Set programResetPin to high impedance and default its value to HIGH.
	pinMode(programResetPin, INPUT);
	digitalWrite(programResetPin, HIGH);

#ifdef DEBUG
	Serial.begin(115200);	// Start serial communication at 115200 baud
#endif

	pinMode(PIN_LED, OUTPUT);
	pinMode(PIN_PUMP, OUTPUT);
	setupWifi.setupWifi();

	// Log in to mqtt server
	loginToMQTT();

	// SPI: Setup the Slave Select Pins as outputs
	pinMode(slaveSelectPin, OUTPUT);
	digitalWrite(slaveSelectPin, HIGH);
	SPI.begin();
}

#ifdef DEBUG
void startupTest()
{
	static AsyncWait startupTestWait;
	static uint8_t startupTestValue;
	static bool firstTime = true;
	static const unsigned turnOnSeconds = 1;
	static const unsigned nextIterationDuration = 1250;

	bool changed = false;

	if (firstTime) {
		firstTime = false;
		startupTestValue = 0;
	}
}
#endif

void loop()
{
	setupWifi.loopWifi();
	if (!setupWifi.isReadyForProcessing()) return;	// if wifi not ready, don't do any other processing

	if (!pubSubClient.connected()) {
		MilliSec currentMilliSec = millis();
		reconnectToMQTT(currentMilliSec);
	}
	pubSubClient.loop();
}
