// Anteater Electric Racing, 2025
#pragma once

#include <stdint.h>

#define SENSOR_PIN_AMT 3

extern uint16_t sensorPins[SENSOR_PIN_AMT];
extern uint16_t sensorIndex;
extern uint16_t sensorReads[SENSOR_PIN_AMT];

void Sensor_Init();
void threadSensor( void *pvParameters );
