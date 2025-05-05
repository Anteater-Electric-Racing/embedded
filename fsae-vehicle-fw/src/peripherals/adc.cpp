// Anteater Electric Racing, 2025

#include <ADC.h>

#include "adc.h"
#include "./vehicle/can.h"
// #include "callbacks.h"
#include <chrono>
#include <arduino_freertos.h>
// #include <semphr.h>

# ifndef EV_ADC
# define EV_ADC

#define THREAD_ADC_STACK_SIZE 128
#define THREAD_ADC_PRIORITY 1
#define THREAD_ADC_TELEMETRY_PRIORITY 1

// SemaphoreHandle_t xSemaphore;

uint16_t adc0Pins[SENSOR_PIN_AMT_ADC0] = {A0, A1, A2, A3, A4, A5, A6, A7}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
volatile uint16_t adc0Index; // = 0;
volatile uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];

uint16_t adc1Pins[SENSOR_PIN_AMT_ADC1] = {A7, A6, A5, A4, A3, A2, A1, A0}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
volatile uint16_t adc1Index; // = 0;
volatile uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

ADC *adc; // = new ADC();

void ADC_Init() {

    // ADC 0
    adc->adc0->setAveraging(1); // set number of averages
    adc->adc0->setResolution(12); // set bits of resolution
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::ADACK_20); // change the conversion speed
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed

    // ADC 1
    adc->adc1->setAveraging(1); // set number of averages
    adc->adc1->setResolution(12); // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::ADACK_20); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed

    Serial.println("Done initializing ADCs");
}

void threadADC( void *pvParameters );

void ADC_Begin() {
    // xTaskCreate(threadCAN, "threadCAN", THREAD_CAN_STACK_SIZE, NULL, THREAD_CAN_PRIORITY, NULL);
    xTaskCreate(threadADC, "threadTelemetryCAN", THREAD_ADC_STACK_SIZE, NULL, THREAD_ADC_PRIORITY, NULL);
}

void threadADC( void *pvParameters ){
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = 1;

    while(true){
        vTaskDelayUntil(&lastWakeTime, xFrequency);
        for(uint16_t currentIndexADC0 = 0; currentIndexADC0 < SENSOR_PIN_AMT_ADC0; ++currentIndexADC0){
            uint16_t currentPinADC0 = adc0Pins[currentIndexADC0];
            uint16_t adcRead = 1; //adc->adc1->analogRead(currentPinADC0); // in callbacks.h
            // ad->startSynchronizedSingleRead
            // uint16_t adcRead = currentIndexADC0; // For testing TODO change
            adc0Reads[currentIndexADC0] = adcRead;
        }

        for(uint16_t currentIndexADC1 = 0; currentIndexADC1 < SENSOR_PIN_AMT_ADC1; ){
            uint16_t currentPinADC1 = adc1Pins[currentIndexADC1];
            uint16_t adcRead = 2; //adc->adc1->analogRead(currentPinADC1); // in callbacks.h
            // uint16_t adcRead = currentIndexADC1; // For testing TODO change
            adc0Reads[currentIndexADC1] = adcRead;
            ++currentIndexADC1;
        }

        CAN_UpdateTelemetryADCData(adc0Reads, adc1Reads);
    }
}

# endif
