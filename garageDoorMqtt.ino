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
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>

#ifndef STASSID
#define STASSID "wifi_ssid"
#define STAPSK  "wifi_password"
#endif

#define HOST "garageDoor"

#define MQTT_SERVER "mqttServer"
#define MQTT_SERVERPORT 1883
#define MQTT_USERNAME "mqtt_username"
#define MQTT_PASSWORD "mqtt_password"
#define MQTT_CLIENTID "garageDoor0"
#define MQTT_DOOR_PUBLISH_TOPIC "home/garagedoor/doorStatus"
#define MQTT_CAR_PUBLISH_TOPIC "home/garagedoor/carStatus"
#define MQTT_SUBSCRIBE_TOPIC "home/garagedoor/trigger"

#define UPDATE_PATH "/firmware"
#define UPDATE_USERNAME "username"
#define UPDATE_PASSWORD "password"

#define DOOR_SENSOR_CLOSED 13
#define DOOR_SENSOR_OPENED 12
#define CAR_SENSOR_TRIGGER 14
#define CAR_SENSOR_ECHO 4
#define DOOR_TRIGGER 2

// Threshold for the distance sensor (in centimeter) lower = car is in garage, higher or equal = car is not in garage
#define CAR_SENSOR_DISTANCE_THESHOLD 1000

//Static IP address configuration
IPAddress staticIP(192, 168, 1, 40);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);

#define DOOR_STATUS_INIT "initializing"
#define DOOR_STATUS_OPEN "open"
#define DOOR_STATUS_CLOSING "closing"
#define DOOR_STATUS_CLOSED "closed"
#define DOOR_STATUS_OPENING "opening"
#define DOOR_STATUS_ERROR "error"

const char* ssid = STASSID;
const char* password = STAPSK;

char* doorStatus = DOOR_STATUS_INIT;
bool carInGarage = false;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient espClient;
PubSubClient client(espClient);

void handleRoot() {
  httpServer.send(200, "text/html", "<html><head></head><body><h2>hello from " + WiFi.hostname() +"!</h2>Door status:" + doorStatus + "</br>Still need to work on a better status report</br><a href=\"/firmware\">Update the firmware here.</a></body></html>");
}

void setup() {
  pinMode(DOOR_TRIGGER, OUTPUT);
  digitalWrite(DOOR_TRIGGER, 0);

  WiFi.hostname(HOST);
  
  Serial.begin(115200);

  pinMode(CAR_SENSOR_TRIGGER, OUTPUT);
  digitalWrite(CAR_SENSOR_TRIGGER, 0);
  pinMode(DOOR_SENSOR_OPENED, INPUT);
  pinMode(DOOR_SENSOR_CLOSED, INPUT);
  pinMode(CAR_SENSOR_ECHO, INPUT);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(staticIP, gateway, subnet, dns);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
  }
  Serial.println();
  Serial.println(F("WiFi connected"));

  MDNS.begin(HOST);
  httpUpdater.setup(&httpServer, UPDATE_PATH, UPDATE_USERNAME, UPDATE_PASSWORD);
  httpServer.on("/", handleRoot);
  
  httpServer.begin();
  Serial.println(F("Server started"));
  Serial.println(WiFi.localIP());

  client.setServer(MQTT_SERVER, MQTT_SERVERPORT);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void handleDoorStatus() {
  boolean doorClosed = digitalRead(DOOR_SENSOR_CLOSED) == LOW;
  boolean doorOpen = digitalRead(DOOR_SENSOR_OPENED) == LOW;
  char* newStatus = doorStatus;
  if (doorClosed && doorOpen) {
    newStatus = DOOR_STATUS_ERROR;
  } else if (doorStatus == DOOR_STATUS_OPEN && !doorOpen) {
    if (doorClosed)
      newStatus = DOOR_STATUS_CLOSED;
    else
      newStatus = DOOR_STATUS_CLOSING;
  } else if (doorStatus == DOOR_STATUS_CLOSED && !doorClosed) {
    if (doorOpen)
      newStatus = DOOR_STATUS_OPEN;
    else
      newStatus = DOOR_STATUS_OPENING;
  } else if (doorClosed) {
    newStatus = DOOR_STATUS_CLOSED;
  } else if (doorOpen) {
    newStatus = DOOR_STATUS_OPEN;
  }

  if (doorStatus != newStatus) {
    doorStatus = newStatus;
    client.publish(MQTT_DOOR_PUBLISH_TOPIC, doorStatus);
  }
}

void handleCarStatus() {
  digitalWrite(CAR_SENSOR_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(CAR_SENSOR_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(CAR_SENSOR_TRIGGER, LOW);
  float duration = pulseIn(CAR_SENSOR_ECHO, HIGH);
  float distance = (duration / 2) * 0.0343;

  if (carInGarage && distance >= CAR_SENSOR_DISTANCE_THESHOLD) {
    carInGarage = false;
    client.publish(MQTT_CAR_PUBLISH_TOPIC, "no");
  } else if (!carInGarage && distance < CAR_SENSOR_DISTANCE_THESHOLD) {
    carInGarage = true;
    client.publish(MQTT_CAR_PUBLISH_TOPIC, "yes");
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
 
  httpServer.handleClient();
  MDNS.update();

  handleDoorStatus();
  handleCarStatus();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.publish(MQTT_DOOR_PUBLISH_TOPIC, doorStatus);
      client.publish(MQTT_CAR_PUBLISH_TOPIC, carInGarage ? "yes" : "no");
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
