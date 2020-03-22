/*
 * Garage door opener over MQTT (using esp8266)

The MIT License (MIT)

Copyright (c) 2018 Simon Vos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include "GarageSettings.h"
#include "DoorStatus.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

char* doorStatus;

bool carInGarage = false;

bool doorTriggered = false;
long doorTriggerTime = 0;
long doorTriggerInterval = DOOR_TRIGGER_INTERVAL;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient espClient;
PubSubClient client(espClient);

void handleRoot() {
  httpServer.send(200, "text/html", "<html><head></head><body><h2>hello from " + WiFi.hostname() +"!</h2>Signal strength:" + WiFi.RSSI() + "<br>Door status:" + doorStatus + "</br>Still need to work on a better status report</br><a href=\"/firmware\">Update the firmware here.</a></body></html>");
}

void setup() {
  Serial.begin(115200);

  doorSetup();
  carSetup();
  setupWifi();

  httpUpdater.setup(&httpServer, FIRMWARE_UPDATE_PATH, FIRMWARE_UPDATE_USERNAME, FIRMWARE_UPDATE_PASSWORD);
  httpServer.on("/", handleRoot);
  
  httpServer.begin();
  Serial.println(F("Server started"));
  Serial.println(WiFi.localIP());

  mqttSetup();
}

void loop() {
  mqttLoop();
  httpServer.handleClient();
  doorLoop();
  carLoop();
  sendStatusInfo();
}

long statusReportPreviousTime = millis();
long statusReportInterval = STATUS_REPORT_INTERVAL;

void sendStatusInfo() {
  if (statusReportInterval > 0 && millis() - statusReportPreviousTime > statusReportInterval) {
    statusReportPreviousTime = millis();
    String rssis = "Wifi Signal: ";
    rssis = rssis + WiFi.RSSI();
    client.publish(MQTT_STATUS_TOPIC, const_cast<char*>(rssis.c_str()));
  }
}
