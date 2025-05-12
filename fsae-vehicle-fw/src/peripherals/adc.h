// Anteater Electric Racing, 2025
#ifndef ADC_EV_H
#define ADC_EV_H

#include <ADC.h>
#define SENSOR_PIN_AMT_ADC0 8
#define SENSOR_PIN_AMT_ADC1 8

extern uint16_t adc0Pins[SENSOR_PIN_AMT_ADC0];
extern volatile uint16_t adc0Index;
extern volatile uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];

extern uint16_t adc1Pins[SENSOR_PIN_AMT_ADC1];
extern volatile uint16_t adc1Index;
extern volatile uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

extern ADC *adc;

void ADC_Init();
void ADC_Begin();

#endif // ADC_EV_H
