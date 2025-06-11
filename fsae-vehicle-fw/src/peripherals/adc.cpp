// Anteater Electric Racing, 2025

#include <ADC.h>
#include <stdint.h>
#include "adc.h"
#include "./vehicle/telemetry.h"
#include <chrono>
#include <arduino_freertos.h>
#include "utils/utils.h"
#include "vehicle/bse.h"
#include "vehicle/apps.h"
#include "vehicle/faults.h"
#include "vehicle/telemetry.h"
#include "vehicle/motor.h"


enum SensorIndexesADC0 { // TODO: Update with real values
    APPS_1_INDEX,
    APPS_2_INDEX,
    BSE_1_INDEX,
    BSE_2_INDEX,
    SUSP_TRAV_LINPOT1,
    SUSP_TRAV_LINPOT2,
    SUSP_TRAV_LINPOT3,
    SUSP_TRAV_LINPOT4
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

uint16_t adc0Pins[SENSOR_PIN_AMT_ADC0] = {A0, A1, A2, A3, A4, A5, A6, A7}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];

uint16_t adc1Pins[SENSOR_PIN_AMT_ADC1] = {A7, A6, A5, A4, A3, A2, A1, A0}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

static TickType_t lastWakeTime;

ADC *adc = new ADC();

void ADC_Init() {
    // ADC 0
    adc->adc0->setAveraging(ADC_AVERAGING); // set number of averages
    adc->adc0->setResolution(ADC_RESOLUTION); // set bits of resolution
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed

    // ADC 1
    adc->adc1->setAveraging(ADC_AVERAGING); // set number of averages
    adc->adc1->setResolution(ADC_RESOLUTION); // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed

    # if DEBUG_FLAG
        Serial.println("Done initializing ADCs");
    # endif
}

void threadADC( void *pvParameters ){
    # if DEBUG_FLAG
        Serial.print("Beginning adc thread");
    # endif

    lastWakeTime = xTaskGetTickCount();
    while(true){
        vTaskDelayUntil(&lastWakeTime, TICKTYPE_FREQUENCY);
        for(uint16_t currentIndexADC0 = 0; currentIndexADC0 < SENSOR_PIN_AMT_ADC0; ++currentIndexADC0){
            uint16_t currentPinADC0 = adc0Pins[currentIndexADC0];
            uint16_t adcRead = adc->adc1->analogRead(currentPinADC0);
            adc0Reads[currentIndexADC0] = adcRead;
        }

        for(uint16_t currentIndexADC1 = 0; currentIndexADC1 < SENSOR_PIN_AMT_ADC1; ++currentIndexADC1){
            uint16_t currentPinADC1 = adc1Pins[currentIndexADC1];
            uint16_t adcRead = adc->adc1->analogRead(currentPinADC1);
            adc1Reads[currentIndexADC1] = adcRead;
        }

        // Update each sensors data
        APPS_UpdateData(adc0Reads[APPS_1_INDEX], adc0Reads[APPS_2_INDEX]);
        BSE_UpdateData(adc0Reads[BSE_1_INDEX], adc0Reads[BSE_2_INDEX]);

        // Handle any faults that were raised
        Faults_HandleFaults();
        Motor_UpdateMotor();
    }
}
