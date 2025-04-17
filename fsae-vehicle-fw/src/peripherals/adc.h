// Anteater Electric Racing, 2025

#ifndef ADC_EV_H
#define ADC_EV_H

#include <ADC.h>
// #include <stdint.h>
#define SENSOR_PIN_AMT_ADC0 8
#define SENSOR_PIN_AMT_ADC1 8

// note: change inputs for potentiometers to A0 (14) and A1 (15)
extern uint16_t adc0Pins[SENSOR_PIN_AMT_ADC0]; // = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
extern volatile uint16_t adc0Index; // = 0;
extern volatile uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];

extern uint16_t adc1Pins[SENSOR_PIN_AMT_ADC1]; // = {0xA7, 0xA6, 0xA5, 0xA4, 0xA3, 0xA2, 0xA1, 0xA0}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
extern volatile uint16_t adc1Index; // = 0;
extern volatile uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

extern ADC *adc; // = new ADC();

void ADC1_Init();

#endif // ADC_H
