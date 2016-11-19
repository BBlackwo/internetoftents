#include <Arduino.h>
#include <FS.h>
#include <stdio.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ESP_MQTTLogger.h>

#include "peripherals.h"
#include "noperipheral.h"
#include "temp_peripheral.h"
#include "baro_peripheral.h"
#include "dht_peripheral.h"
#include "servo_peripheral.h"


#include "./node_modes.h"
bool _state_config = false;
uint8_t _sleep_period = DEFAULT_SLEEP_MINS;

#define DEBUG true

// we use this to determine the minimum period between publish messages.
// This is so that if we're "always on" then we won't keep spamming the MQTT
// server with messages
#define MIN_PUBLISH_PERIOD (60 * 1000) // 60s default

unsigned long _lastpublish = 0;
unsigned long _nextpublish = 0;

Peripheral * _vcc_sensor;
Peripheral * _device_peripheral;

MODES _mode = NONE;

void setup_node_peripherals(ESP_MQTTLogger& l) {
    // here we take the mode and we set up the sensors we're going to be using.

#ifdef DEBUG
    Serial.println("Setting up node peripherals");
#endif

    _vcc_sensor = new NoPeripheral();
    _vcc_sensor->begin(l);

    // choose which mode we're in and set up peripheral appropriately.
    switch (_mode) {

        case NONE:
            break;

        case TEMP_1WIRE:
            _device_peripheral = new TempPeripheral();
            break;

        case BARO:
            _device_peripheral = new BaroPeripheral();
            break;

        case DHT:
            _device_peripheral = new DHTPeripheral();
            break;
    }

    if (_mode > NONE) {
        _device_peripheral->begin(l);
    }
}

void publish_peripheral_data() {
    // publish whatever the peripherals are.

    // do a check on timing for publish
    // we do a zero val check here to get a publish on wakeup.
    if (millis() > _nextpublish) {
#ifdef DEBUG
        Serial.println("Publishing peripheral data");
#endif

        _vcc_sensor->publish_data();

        if (_mode > NONE) {
            _device_peripheral->publish_data();
        }
        _lastpublish = millis();
        _nextpublish = _lastpublish + MIN_PUBLISH_PERIOD;
    } else {
        ;
    }
}

void config_subscription(char* topic, byte* payload, unsigned int length) {

    // handles the things around the subscription.
    //
    String t = (String)topic;

    // grab the data from the payload and get it in a form we can use it.
    //

    char buf[length + 1];
    for (int i = 0; i < length; i++) {
        buf[i] = (char)payload[i];
    }
    buf[length] = '\0';
    String p = String(buf);

#ifdef DEBUG
    Serial.print("Topic: ");
    Serial.print(t);
    Serial.print(" Payload: ");
    Serial.println(p);
#endif

    if (t.endsWith("sleep")) {
        set_sleep_time(p.toInt());
    } else if (t.endsWith("mode")) {
        set_mode(p);
    }
}

bool state_configured() {
    return _state_config;
}

uint8_t get_sleep_time() {

    return _sleep_period;
}

void set_sleep_time(uint8_t sleep) {
    // here we set how long the system should hibernate for

    _sleep_period = sleep;
    // TODO Write to the file system.
}

MODES get_mode() {
    return _mode;
}

void set_mode(String modename) {

    if (! _state_config) {
        _mode = NONE;

        if (modename == "1wiretemp") {
            _mode = TEMP_1WIRE;
        } else if (modename == "baro") {
            _mode = BARO;
        } else if (modename == "dht") {
            _mode = DHT;
        }

        _state_config = true;
    } else {
        // DEAL WITH A MODE CHANGE
        // The best thing to do here is actually trigger a
        // deep sleep reset as it will come back up in the different
        // mode with all the appropriate objects set up etc.
        Serial.println("Mode reconfigure. Reset 1 sec");
        ESP.deepSleep(1 * 1000l * 1000l);
    }

}
bool save_config(String filename, String value) {
    // takes a filename and a string and writes it down.
    File configFile = SPIFFS.open(filename, "w");

    if (!configFile) {
        Serial.println("Error saving to file system");
        return false;
    }

    configFile.print(value);
    return true;
}

String load_config(String filename) {
    // loads a file and returns its contents

    File configFile = SPIFFS.open(filename, "r");

    if (configFile) {
        return configFile.readString();
    }

    Serial.println("Error reading from file");
    return "";
}
