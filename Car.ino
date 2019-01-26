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

void carSetup() {
  pinMode(CAR_SENSOR_TRIGGER, OUTPUT);
  digitalWrite(CAR_SENSOR_TRIGGER, 0);
  pinMode(CAR_SENSOR_ECHO, INPUT);

}

void carLoop() {
  digitalWrite(CAR_SENSOR_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(CAR_SENSOR_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(CAR_SENSOR_TRIGGER, LOW);
  float duration = pulseIn(CAR_SENSOR_ECHO, HIGH);
  float distance = (duration / 2) * 0.0343;

  if (carInGarage && distance >= CAR_SENSOR_DISTANCE_THESHOLD) {
    carInGarage = false;
    client.publish(MQTT_CAR_STATUS_TOPIC, "no");
  } else if (!carInGarage && distance < CAR_SENSOR_DISTANCE_THESHOLD) {
    carInGarage = true;
    client.publish(MQTT_CAR_STATUS_TOPIC, "yes");
  }
}
