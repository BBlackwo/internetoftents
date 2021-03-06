#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ESP_MQTTLogger.h>

#include "pixel_peripheral.h"

#define PIXEL_PIN  2
#define PIXEL_COUNT 64

// corresponds to 10fps which is fast enough for static displays
#define FRAME_MILLIS 30

PixelPeripheral::PixelPeripheral() {}

// ESP8266 show() is external to enforce ICACHE_RAM_ATTR execution
extern "C" void ICACHE_RAM_ATTR show(
  uint8_t pin, uint8_t *pixels, uint32_t numBytes);

void PixelPeripheral::set_pixel(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b) {
    // set the pixel value in memory

    uint16_t pa = pixel * _colour_depth;
    // assuming GRB addressing with this for speed
    _px[pa] = g;
    _px[pa+1] = r;
    _px[pa+2] = b;
}

void PixelPeripheral::set_strip(uint8_t r, uint8_t g, uint8_t b) {
    // sets the entire strip this colour.
    for (uint16_t i=0; i < _px_count; i++) {
        set_pixel(i, r, g, b);
    }
}

void PixelPeripheral::initialise_pixels(uint16_t num_pixels) {
    // set up the pixel array, allocate mem etc.
    //
    if (_px) {
        free (_px);
        _px_count = 0;
    }

    if (num_pixels > 0) {
        if (_px = (uint8_t *)malloc(num_pixels * _colour_depth)) {
            memset(_px, 0, num_pixels * _colour_depth);
            _px_count = num_pixels;
        } else {
            _px_count = 0;
        }
    }

    pinMode(PIXEL_PIN, OUTPUT);

    Serial.print("Init pixels. Pixel count: ");
    Serial.println(_px_count);
}

void PixelPeripheral::begin(ESP_MQTTLogger& l) {
    // set up the logger
    _logger = l;

    _updateable = true;
    _publishable = false;

    initialise_pixels(PIXEL_COUNT);

    bool subbed = _logger.subscribe("ic/#");

    if (! subbed) {
        Serial.println("Couldn't subscribe to content messages");
        _logger.publish("sys/error", "sub_fail");
    } else {
        _logger.publish("oc/status", "available");
    }
}

void PixelPeripheral::publish_data() {

    // for a pixel display - this is a NOOP
}

void PixelPeripheral::sub_handler(String topic, String payload) {

    // bust up the string so we can grab the pertinent parts.
    char s[256];
    strcpy(s, topic.c_str());
    char* token = strtok(s, "/");

    uint16_t pixel_number;
    uint8_t r, g, b;

    PX_STATES state = PXP_START;
    const char* cmp = "px";

    // we can drop the first as it's only going to be the ID.
    while (token) {
        token = strtok(NULL, "/");
        String t = String(token);

        switch (state) {

            case PXP_START:
                // see if our token is px or strip
                if (t == "px") {
                    state = PXP_PIXEL;
                } else if (t == "strip") {
                    state = PXP_STRIP;
                } else if (t == "data") {
                    state = PXP_DATA;
                }
                continue;
                break;

            case PXP_PIXEL:
                // the next token should be a number
                // explicitly test for 0
                if (t == "0") {
                    pixel_number = 0;
                } else {
                    pixel_number = t.toInt();
                    if (pixel_number == 0) {
                        // there was an error
                        pixel_number = NULL;
                    }
                }

                // get the pixel values off the payload and turn them into
                // rgb vals
                r = g = b = 0;

                // convert to values which are then put into the pixel array.
                if (payload.length() != 6) {
                    Serial.print("PXP: Error pixel value incorrect: ");
                    Serial.println(payload);
                } else {
                    // we're good so peel the numbers off and let's go.
                    String v = payload.substring(0, 2);
                    r = (uint8_t)strtol(v.c_str(), NULL, 16);
                    v = payload.substring(2, 4);
                    g = (uint8_t)strtol(v.c_str(), NULL, 16);
                    v = payload.substring(4);
                    b = (uint8_t)strtol(v.c_str(), NULL, 16);

                    if (pixel_number != NULL) {
                        set_pixel(pixel_number, r, g, b);
                    }
                }

                break;  // PXP_PIXEL

            case PXP_STRIP:
                r = g = b = 0;

                // convert to values which are then put into the pixel array.
                if (payload.length() != 6) {
                    Serial.print("PXP: Error pixel value incorrect: ");
                    Serial.println(payload);
                } else {
                    // we're good so peel the numbers off and let's go.
                    String v = payload.substring(0, 2);
                    r = (uint8_t)strtol(v.c_str(), NULL, 16);
                    v = payload.substring(2, 4);
                    g = (uint8_t)strtol(v.c_str(), NULL, 16);
                    v = payload.substring(4);
                    b = (uint8_t)strtol(v.c_str(), NULL, 16);

                    set_strip(r, g, b);

                }
                break;

            case PXP_DATA:

                // we have here a buffer of pixel data.
                // reset to zeros
                uint16_t px_bytes = _px_count * _colour_depth;
                uint16_t cpy_len = px_bytes;

                memset(_px, 0, px_bytes);
                // now overwrite the px buffer

                if (payload.length() < px_bytes) {
                    cpy_len = payload.length();
                }
                memcpy(_px, payload.c_str(), cpy_len);
                break;
        }
    }

}

void PixelPeripheral::update() {

    // update the state of the pixels
    //
    // we do this to fps lock the pixels. The reason is that
    // update is called as fast as possible and it can sometimes
    // cause issues with flicker.
    if (millis() > (_last_update + FRAME_MILLIS) ) {
        show(PIXEL_PIN, _px, _px_count * _colour_depth);
        _last_update = millis();
    }
}

