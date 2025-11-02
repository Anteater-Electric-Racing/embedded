// Anteater Electric Racing, 2025

#include "imu.h"

#include <arduino_freertos.h>


template <typename T>
T constrain(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

uint16_t sensorPins[SENSOR_PIN_AMT] = {A0, A1, A2};
uint16_t sensorIndex;
uint16_t sensorReads[SENSOR_PIN_AMT];

void threadSensor(void *pvParameters) {

    while(true) {
        // Read sensors here
        for (int i = 0; i < SENSOR_PIN_AMT; i++) {
            sensorReads[i] = analogRead(sensorPins[i]);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10)); // 100 Hz

    }
}

void Sensor_Init() {
}