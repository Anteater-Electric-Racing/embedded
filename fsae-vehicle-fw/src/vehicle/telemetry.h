// Anteater Electric Racing, 2025

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <peripherals/adc.h>

struct TelemetryData{
    float accumulatorVoltage;
    float accumulatorTemp;

    float tractiveSystemVoltage;
    uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];
    uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

} __attribute__((packed)); // need this to ensure data is packed without gaps in between;


void Telemetry_Init();
TelemetryData* Telemetry_GetData();
void Telemetry_UpdateData(TelemetryData* data);
void Telemetry_UpdateADCData(volatile uint16_t* adc0reads, volatile uint16_t* adc1reads);


#endif
