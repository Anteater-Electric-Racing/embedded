#include <Arduino.h>

static const uint8_t DELAY = 8; // ms
static const uint8_t READ_PINS[] = {A0, A1, A2,
                                    A3}; // brakeA, brakeB, shockA, shockB
static const uint8_t WRITE_PINS[] = {};

void setup() {
    Serial.begin(9600);
    for (uint8_t pin : WRITE_PINS) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
}

void loop() {
    for (uint8_t i = 0; i < sizeof(READ_PINS); ++i) {
        int val = analogRead(READ_PINS[i]);
        Serial.print(val);
        if (i < sizeof(READ_PINS) - 1) {
            Serial.print(' ');
        }
    }
    Serial.println();
    delay(DELAY);
}