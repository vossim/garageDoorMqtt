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

long mqttReconnectTimestamp = 0;

void mqttSetup() {
  client.setServer(MQTT_SERVER, MQTT_SERVERPORT);
  client.setCallback(mqttCallback);
}

void mqttLoop() {
  if (!client.connected()) {
    mqttReconnect();
  } else {
    client.loop();  
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  doorTrigger();
}

void mqttReconnect() {
  long now = millis();
  if (now - mqttReconnectTimestamp > 5000) {
    mqttReconnectTimestamp = now;
    // Attempt to reconnect
    if (client.connect(MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("MQTT connected");
      mqttReconnectTimestamp = 0;
      client.publish(MQTT_DOOR_STATUS_TOPIC, doorStatus);
      client.publish(MQTT_CAR_STATUS_TOPIC, carInGarage ? "yes" : "no");
      client.subscribe(MQTT_DOOR_COMMAND_TOPIC);
    }
  }
}
