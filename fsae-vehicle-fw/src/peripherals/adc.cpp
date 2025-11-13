// Anteater Electric Racing, 2025

#include "adc.h"
#include "./vehicle/telemetry.h"
#include "utils/utils.h"
#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "vehicle/motor.h"
#include "vehicle/telemetry.h"
#include "vehicle/thermal.h"
#include <ADC.h>
#include <arduino_freertos.h>
#include <chrono>
#include <stdint.h>

enum SensorIndexesADC0 {    // TODO: Update with real values
    THERMISTOR_1_INDEX = 0, // A0
    APPS_1_INDEX = 5,
    APPS_2_INDEX = 6, // A6
    BSE_1_INDEX = 3,
    BSE_2_INDEX = 2,
    SUSP_TRAV_LINPOT1,
    SUSP_TRAV_LINPOT2,
    SUSP_TRAV_LINPOT3,
    SUSP_TRAV_LINPOT4,
    THERMISTOR_2_INDEX = 10, // A1
    THERMISTOR_3_INDEX = 9,  // A2
    THERMISTOR_4_INDEX = 8   // A3
};

enum SensorIndexesADC1 { // TODO: Update with real values
    APPS_1_INDEX2,
    APPS_2_INDEX2,
    BSE_1_INDEX2,
    BSE_2_INDEX2,
    SUSP_TRAV_LINPOT12,
    SUSP_TRAV_LINPOT22,
    SUSP_TRAV_LINPOT32,
    SUSP_TRAV_LINPOT42
};

uint16_t adc0Pins[SENSOR_PIN_AMT_ADC0] = {
    A0, A1, A2,  A3,  A4, A5,
    A6, A7, A15, A16, A17}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21,
                            // 24, 25, 19, 18, 14, 15, 17};
uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];

uint16_t adc1Pins[SENSOR_PIN_AMT_ADC1] = {
    A17, A16, A15, A7, A6, A5,
    A4,  A3,  A2,  A1, A0}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21,
                            // 24, 25, 19, 18, 14, 15, 17};
uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

static TickType_t lastWakeTime;

ADC *adc = new ADC();

void ADC_Init() {
    // ADC 0
    adc->adc0->setAveraging(ADC_AVERAGING);   // set number of averages
    adc->adc0->setResolution(ADC_RESOLUTION); // set bits of resolution
    adc->adc0->setConversionSpeed(
        ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc0->setSamplingSpeed(
        ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed

    // ADC 1
    adc->adc1->setAveraging(ADC_AVERAGING);   // set number of averages
    adc->adc1->setResolution(ADC_RESOLUTION); // set bits of resolution
    adc->adc1->setConversionSpeed(
        ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc1->setSamplingSpeed(
        ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed

#if DEBUG_FLAG
    Serial.println("Done initializing ADCs");
#endif
}

void threadADC(void *pvParameters) {
#if DEBUG_FLAG
    Serial.print("Beginning adc thread");
#endif

    lastWakeTime = xTaskGetTickCount();
    while (true) {
        vTaskDelayUntil(&lastWakeTime, TICKTYPE_FREQUENCY);
        for (uint16_t currentIndexADC0 = 0;
             currentIndexADC0 < SENSOR_PIN_AMT_ADC0; ++currentIndexADC0) {
            uint16_t currentPinADC0 = adc0Pins[currentIndexADC0];
            uint16_t adcRead = adc->adc1->analogRead(currentPinADC0);
            adc0Reads[currentIndexADC0] = adcRead;
        }

        for (uint16_t currentIndexADC1 = 0;
             currentIndexADC1 < SENSOR_PIN_AMT_ADC1; ++currentIndexADC1) {
            uint16_t currentPinADC1 = adc1Pins[currentIndexADC1];
            uint16_t adcRead = adc->adc1->analogRead(currentPinADC1);
            adc1Reads[currentIndexADC1] = adcRead;
        }

        // Update each sensors data
        APPS_UpdateData(adc0Reads[APPS_1_INDEX], adc0Reads[APPS_2_INDEX]);
        BSE_UpdateData(adc0Reads[BSE_1_INDEX], adc0Reads[BSE_2_INDEX]);

        thermal_Update(
            adc0Reads[THERMISTOR_1_INDEX], adc0Reads[THERMISTOR_2_INDEX],
            adc0Reads[THERMISTOR_3_INDEX], adc0Reads[THERMISTOR_4_INDEX]);

        // Handle any faults that were raised
        // Faults_HandleFaults();
        // Motor_UpdateMotor();
    }
}
