# Garage door opener over MQTT (using esp8266)
Arduino based project, using an esp8266, to control a standard garage door opener over MQTT.

## Description
An arduino project which aims to add a bit of smarts to a standard garage door opener. The final solution should detect the state the garage door is in (open/closed/opening/closing), trigger the door to open or close and detect if a car is present in the garage. All of the connectivity will go over MQTT.

- [x] Garage door status
- [x] Garage door trigger
- [x] Car presence status
- [ ] Secure the MQTT connection

## How to use
1. Open up garageDoor.ino in arduino, and make sure you install the ESP8266 core for Arduino (https://github.com/esp8266/Arduino I'm using 2.5.0-beta2), and the Arduino client for MQTT (https://pubsubclient.knolleary.net/ or https://github.com/knolleary/pubsubclient/ I'm using 2.7.0).
2. Copy `GarageSettings.h.template` to `GarageSettings.h`, and set you wifi, mqtt, sensor pins and "over-the-air" update settings (the OTA settings can be used to update your ESP8266 over the air), and any other settings you want to update.
3. Flash to your ESP8266

## Over-the-air updates
For convenience, the over-the-air update feature from the ESP8266 core for Arduino is integrated. For this you can configure the path which the ESP8266 will listen at for updates, the username and password. You'll need an ESP8266 with at least 1MB of flash to use this feature.

It's recommented to generate a public and private key, and sign your binary. This will ensure your device can only get updated over-the-air by binaries signed with the same private key (for more information see: https://arduino-esp8266.readthedocs.io/en/2.5.0-beta2/ota_updates/readme.html).
