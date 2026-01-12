// Anteater Electric Racing, 2025

#include "adc.h"
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
    APPS_1_INDEX = 4,
    APPS_2_INDEX = 5, // A5
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

struct AdcPinConfig {
    uint8_t pin;
    uint8_t dataIndex;
};

uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];
uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

const AdcPinConfig adc0Pins[SENSOR_PIN_AMT_ADC0] = {
    {A0, THERMISTOR_1_INDEX},  // index 0
    {A1, 1},                   // index 1
    {A2, BSE_2_INDEX},         // index 2
    {A3, BSE_1_INDEX},         // index 3
    {A4, APPS_1_INDEX},        // index 4
    {A5, APPS_2_INDEX},        // index 5
    {A6, SUSP_TRAV_LINPOT1},   // index 6
    {A7, SUSP_TRAV_LINPOT2},   // index 7
    {A9, THERMISTOR_4_INDEX},  // index 8
    {A16, THERMISTOR_3_INDEX}, // index 9
    {A17, THERMISTOR_2_INDEX}, // index 10
};

const AdcPinConfig adc1Pins[SENSOR_PIN_AMT_ADC1] = {
    {A17, APPS_1_INDEX2},     // index 0
    {A16, APPS_2_INDEX2},     // index 1
    {A15, BSE_1_INDEX2},      // index 2
    {A7, BSE_2_INDEX2},       // index 3
    {A6, SUSP_TRAV_LINPOT12}, // index 4
    {A5, SUSP_TRAV_LINPOT22}, // index 5
    {A4, SUSP_TRAV_LINPOT32}, // index 6
    {A3, SUSP_TRAV_LINPOT42}, // index 7
    {A2, 8},
    {A1, 9},
    {A0, 10},
};

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

        // Read ADC 0
        for (uint16_t i = 0; i < SENSOR_PIN_AMT_ADC0; ++i) {
            uint16_t currentPin = adc0Pins[i].pin;
            uint16_t targetIndex = adc0Pins[i].dataIndex;
            adc0Reads[targetIndex] = adc->adc0->analogRead(currentPin);
        }

        // Read ADC 1
        for (uint16_t i = 0; i < SENSOR_PIN_AMT_ADC1; ++i) {
            uint16_t currentPin = adc1Pins[i].pin;
            uint16_t targetIndex = adc1Pins[i].dataIndex;
            adc1Reads[targetIndex] = adc->adc1->analogRead(currentPin);
        }

        // Serial.print("ADC0 Reads: ");
        // for (int i = 0; i < SENSOR_PIN_AMT_ADC0; i++) {
        //     Serial.print(adc0Reads[i]);
        //     Serial.print(" ");
        // }
        // Serial.println();
        // Serial.print("ADC1 Reads: ");
        // for (int i = 0; i < SENSOR_PIN_AMT_ADC1; i++) {
        //     Serial.print(adc1Reads[i]);
        //     Serial.print(" ");
        // }
        // Serial.println();
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
