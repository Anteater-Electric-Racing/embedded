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

template <typename T>
T constrain(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

enum SensorIndexesADC0 { // TODO: Update with real values
    APPS_1_INDEX,
    APPS_2_INDEX,
    BSE_1_INDEX,
    BSE_2_INDEX,
    SUSP_TRAV_LINPOT1,
    SHOCK_TRAVEL_INDEX1,
    STEERING_ANGLE_INDEX,
    STEERING_TORQUE_INDEX
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

uint16_t adc0Pins[SENSOR_PIN_AMT_ADC0] = {A0, A1, A2, A3, A4, A5, A6, A7}; // A4, A4, 18, 17, 17, 17, 17 // real values: {21, 24, 25, 19, 18, 14, 15, 17}
uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];

uint16_t adc1Pins[SENSOR_PIN_AMT_ADC1] = {A7, A6, A5, A4, A3, A2, A1, A0}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

struct sensor_t {
    float shockTravel_mm;
    float steeringAngle_deg;
    float steeringTorque_Nm;
};
static struct sensor_t sensorData;

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

         //Converts to the bit-RES values from raw voltage, which supposedly teensy can't read? UPDATED: Used ADC_VALUE_TO_VOLTAGE macro from utils.h to do the conversion from output to controller
        const float shockVoltage   = ADC_VALUE_TO_VOLTAGE(adc0Reads[SHOCK_TRAVEL_INDEX1]); // Might need to add the otherthree
        const float steerAngleVoltage   = ADC_VALUE_TO_VOLTAGE(adc0Reads[STEERING_ANGLE_INDEX]);
        const float torqueVoltage  = ADC_VALUE_TO_VOLTAGE(adc0Reads[STEERING_TORQUE_INDEX]);

        //Converts those bit-RES values into the actual human-read values;
        sensorData.shockTravel_mm = constrain((shockVoltage / 5.0f) * SHOCK_TRAVEL_MAX_MM, 0.0f, SHOCK_TRAVEL_MAX_MM);
        //Updated to account for the mid range (0 degrees or 0 torque) to be at center of voltage range instead of the minimum
        sensorData.steeringAngle_deg = constrain((steerAngleVoltage / 5.0f) * STEERING_ANGLE_MAX_DEG - (STEERING_ANGLE_MAX_DEG* 0.5f), -STEERING_ANGLE_MAX_DEG * 0.5f, STEERING_ANGLE_MAX_DEG * 0.5f);
        sensorData.steeringTorque_Nm = constrain((torqueVoltage / 5.0f) * TORQUE_SENSOR_MAX_NM - (TORQUE_SENSOR_MAX_NM * 0.5f), -TORQUE_SENSOR_MAX_NM * 0.5f, TORQUE_SENSOR_MAX_NM* 0.5f);

        // Update each sensors data
        APPS_UpdateData(adc0Reads[APPS_1_INDEX], adc0Reads[APPS_2_INDEX]);
        BSE_UpdateData(adc0Reads[BSE_1_INDEX], adc0Reads[BSE_2_INDEX]);

        // Handle any faults that were raised
        // Faults_HandleFaults();
        // Motor_UpdateMotor();
    }

}
float Get_ShockTravel_mm() {
    return sensorData.shockTravel_mm;
}

float Get_SteeringAngle_deg() {
    return sensorData.steeringAngle_deg;
}

float Get_SteeringTorque_Nm() {
    return sensorData.steeringTorque_Nm;
}
