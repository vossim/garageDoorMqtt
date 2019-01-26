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

void doorSetup() {
  pinMode(DOOR_TRIGGER, OUTPUT);
  digitalWrite(DOOR_TRIGGER, 0);
  pinMode(DOOR_SENSOR_OPENED, INPUT);
  pinMode(DOOR_SENSOR_CLOSED, INPUT);
  doorStatus = DOOR_STATUS_INIT;
}

void doorLoop() {
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
    client.publish(MQTT_DOOR_STATUS_TOPIC, doorStatus);
  }
}

void doorTrigger() {
  // TODO: Need to trigger the door here
}
