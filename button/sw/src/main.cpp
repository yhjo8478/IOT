#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

extern "C" {
#include "user_interface.h"
}

#include "Thingplus.h"

const char *ssid = "";
const char *password = "";
const char *apikey = "";

const char *buttonId = "";
const char *batteryId = "";

static WiFiClient wifiClient;

ADC_MODE(ADC_VCC);

static void _serialInit(void)
{
	Serial.begin(115200);
	while (!Serial);// wait for serial port to connect.
	Serial.println();
}

static void _wifiInit(void)
{
#define WIFI_MAX_RETRY 150

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	Serial.print("[INFO] WiFi connecting to ");
	Serial.println(ssid);

	int retry = 0;
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(retry++);
		Serial.print(".");
		if (!(retry % 50))
			Serial.println();
		if (retry == WIFI_MAX_RETRY) {
			Serial.println();
			Serial.print("[ERR] WiFi connection failed. SSID:");
			Serial.print(ssid);
			Serial.print(" PASSWD:");
			Serial.println(password);

			system_deep_sleep(0);
		}

		delay(100);
	}

	Serial.println();
	Serial.print("[INFO] WiFi connected");
	Serial.print(" IP:");
	Serial.println(WiFi.localIP());
}

static void _buttonToggle() {
	Thingplus.sensorStatusPublish(buttonId, true, 60 * 24 * 30);
	int buttonValue = 1;
	Thingplus.valuePublish(buttonId, buttonValue);

	delay(100);
	buttonValue = 0;
	Thingplus.valuePublish(buttonId, buttonValue);
}

static void _batteryGaugeSend() {
	Thingplus.sensorStatusPublish(batteryId, true, 60 * 24 * 30);
	int batteryGauge = ESP.getVcc();

	Thingplus.valuePublish(batteryId, batteryGauge);
}

void setup() {
	system_deep_sleep_set_option(0);

	_serialInit();

	uint8_t mac[6];
	WiFi.macAddress(mac);

	Serial.print("[INFO] Gateway Id:");
	Serial.println(WiFi.macAddress());

	_wifiInit();

	Thingplus.begin(wifiClient, mac, apikey);
	Thingplus.connect();
}

void loop() 
{
	Thingplus.loop();
	Thingplus.loop();

	Thingplus.gatewayStatusPublish(true, 60 * 24 * 30);

	_buttonToggle();
	_batteryGaugeSend();

	Thingplus.loop();

	Thingplus.mqttStatusPublish(false);

	Serial.println("Deep sleep");
	system_deep_sleep(0);
	delay(100);
}
