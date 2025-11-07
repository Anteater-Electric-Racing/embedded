// Anteater Electric Racing, 2025
#pragma once

#include <ADC.h>
#include <stdint.h>

#define SENSOR_PIN_AMT_ADC0 8
#define SENSOR_PIN_AMT_ADC1 8

extern uint16_t adc0Pins[SENSOR_PIN_AMT_ADC0];
extern uint16_t adc0Index;
extern uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];

extern uint16_t adc1Pins[SENSOR_PIN_AMT_ADC1];
extern uint16_t adc1Index;
extern uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];
class ADC;
extern ADC *adc;

void ADC_Init();
void threadADC( void *pvParameters );
float getShockTravel_mm();
float getSteeringAngle_deg();
float getSteeringTorque_Nm();
