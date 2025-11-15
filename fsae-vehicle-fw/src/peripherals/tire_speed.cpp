// Anteater Electric Racing, 2025

#include <Arduino.h>
#include <FreeRTOS.h>
#include <stdint.h>
#include <semphr.h>

#include "tire_speed.h"

struct SpeedData {
    float rpm;
    float speed_mps;
    float speed_kph;
    bool  valid;
};

SpeedData wheelSpeed;
SemaphoreHandle_t speedMutex;

void Tire_Speed_Init() {
    attachInterrupt(digitalPinToInterrupt(HALL_PIN), hallISR, RISING);
}

void hallISR() {
    uint32_t currentMicros = micros();
    pulsePeriodMicros = currentMicros - lastPulseMicros;
    lastPulseMicros = currentMicros;
    pulseCount++;
}

void SpeedTask(void *parameter) {
    const TickType_t period = pdMS_TO_TICKS(50); // 50 ms = 20 Hz update
    TickType_t lastWake = xTaskGetTickCount();

    uint32_t lastCount = 0;

    for (;;) {
        vTaskDelayUntil(&lastWake, period);

        // Atomically read pulse count
        uint32_t count = pulseCount;
        uint32_t newPulses = count - lastCount;
        lastCount = count;

        float freq = newPulses / 0.05f;  // 50ms window → *20 for Hz
        float rpm = (freq / NUM_TEETH) * 60.0f;

        // Low-speed condition: use pulse period instead
        if (newPulses == 0 && pulsePeriodMicros > 0) {
            float periodSec = pulsePeriodMicros / 1e6f;
            float freqLow = 1.0f / periodSec;
            rpm = (freqLow / NUM_TEETH) * 60.0f;
        }

        // Convert to linear speed (example tire circumference)
        float wheelCircumference_m = 2.0f; // Insert real value
        float speed_mps = (rpm / 60.0f) * wheelCircumference_m;
        float speed_kph = speed_mps * 3.6f;

        // Store result safely
        xSemaphoreTake(speedMutex, portMAX_DELAY);
        wheelSpeed.rpm = rpm;
        wheelSpeed.speed_mps = speed_mps;
        wheelSpeed.speed_kph = speed_kph;
        wheelSpeed.valid = (rpm > 0.01f);
        xSemaphoreGive(speedMutex);
    }
}

SpeedData getWheelSpeed() {
    SpeedData copy;
    xSemaphoreTake(speedMutex, portMAX_DELAY);
    copy = wheelSpeed;
    xSemaphoreGive(speedMutex);
    return copy;
}
