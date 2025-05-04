// Anteater Electric Racing, 2025

#ifndef CAN_H
#define CAN_H

#include <peripherals/adc.h>

struct CANTelemetryData{
    float accumulatorVoltage;
    float accumulatorTemp;

    float tractiveSystemVoltage;
    uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];
    uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

} __attribute__((packed)); // need this to ensure data is packed without gaps in between;

struct CANADCValues{
    uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];
    uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];
} __attribute__((packed));

void CAN_Init();
void CAN_Begin();
void CAN_TelemetryInit();
void CAN_SerializeTelemetryData(CANTelemetryData data, uint8_t* serializedTelemetryBuf);
void CAN_UpdateTelemetryData(CANTelemetryData* data);
void CAN_UpdateAccumulatorData(float accumulatorVoltage, float accumulatorTemp, float tractiveSystemVoltage);
void CAN_UpdateTelemetryADCData(volatile uint16_t* adc0reads, volatile uint16_t* adc1reads);

#endif
