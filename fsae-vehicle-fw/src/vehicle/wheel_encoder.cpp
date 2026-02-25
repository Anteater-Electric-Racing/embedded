#include "wheel_encoder.h"

const int wheel_encoder_pin = 2; // Replace with correct value
static const int GEAR_TEETH = 32; // Replace with correct value
static const uint32_t TIMEOUT_MICROS = 500000; //If no tooth is seen in 0.5 seconds rpm is zero.


volatile uint32_t lastPulseTime = 0;
volatile uint32_t microDelta = 0;
float currentRPM = 0.0;


void countPulse() {
    uint32_t now = micros();
    microDelta = now - lastPulseTime;
    lastPulseTime = now;
}

void Wheel_Encoder_Init() {
    pinMode(wheel_encoder_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(wheel_encoder_pin), countPulse, FALLING);
}

void Wheel_Encoder_Update() {
    uint32_t delta;
    uint32_t lastTime;
    uint32_t now = micros();


    noInterrupts();
    delta = microDelta;
    lastTime = lastPulseTime;
    interrupts();

    if (now - lastTime > TIMEOUT_MICROS) {
        currentRPM = 0;
    }
    else if (delta > 0) {
        currentRPM = 60000000.0f / (float)(delta * GEAR_TEETH);
    }
}

float getWheelRPM() {
    return currentRPM;
}
