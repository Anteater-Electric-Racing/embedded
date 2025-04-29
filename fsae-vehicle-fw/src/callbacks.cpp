// Anteater Electric Racing, 2025

#include <ADC.h>
#include <stdint.h>

#include "callbacks.h"
#include "peripherals/adc.h"
#include "utils/utils.h"

#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "vehicle/telemetry.h"
#include "vehicle/motor.h"

#define LOGIC_LEVEL_V 3.3
#define ADC_RESOLUTION 10
#define adc0_MAX_VALUE (1 << ADC_RESOLUTION) - 1

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

void StartADCScanCallback() {
    noInterrupts();
    if (adc0Index != 0 || adc1Index != 0){ // means it was not reset at the end of the callback cycle: means callbacks did not complete
        _reboot_Teensyduino_();
    }
    adc0Index = 0;
    adc1Index = 0;
    uint16_t startPinADC0 = adc0Pins[0];
    uint16_t startPinADC1 = adc1Pins[0];
    uint32_t currentTime = micros();
    adc->adc0->enableInterrupts(ADCConversionCompleteCallback);
    adc->startSynchronizedSingleRead(startPinADC0, startPinADC1); // in callbacks.h
    interrupts();
}

void ADCConversionCompleteCallback () {
    // read from pin
    // store pin reads
    // go to next pin
    noInterrupts();

    // if both are less than the range, read both
    //    if next index is in both ranges, start sync read
    //    if next index in 0 range, start read
    //    if next index in 1 range, start read
    // if 0 is less than the range and 1 is out of range, read 0 and start 0
    // if 1 is less than the range and 0 is out of range, read 1 and start 1
    // if both out of range, make both indexes to 0 and end

    if(adc0Index <= SENSOR_PIN_AMT_ADC0 && adc1Index <= SENSOR_PIN_AMT_ADC1){
        ADC::Sync_result result = adc->readSynchronizedSingle();
        uint16_t sensorReadADC0 = result.result_adc0;
        uint16_t sensorReadADC1 = result.result_adc1;

        adc0Reads[adc0Index] = sensorReadADC0; //*LOGIC_LEVEL_V/adc0_MAX_VALUE; // Get value within 0V to 3.3V range
        adc1Reads[adc1Index] = sensorReadADC1;

        ++adc0Index;
        ++adc1Index;
        if (adc0Index < SENSOR_PIN_AMT_ADC0 && adc1Index < SENSOR_PIN_AMT_ADC1){
            adc->startSynchronizedSingleRead(adc0Pins[adc0Index], adc1Pins[adc1Index]);
        }
    } else if (adc0Index <= SENSOR_PIN_AMT_ADC0){
        uint16_t sensorReadADC0 = adc->adc0->readSingle();
        adc0Reads[adc0Index] = sensorReadADC0;
        ++adc0Index;
    } else if (adc1Index <= SENSOR_PIN_AMT_ADC1){
        uint16_t sensorReadADC1 = adc->adc1->readSingle();
        adc1Reads[adc1Index] = sensorReadADC1;
        ++adc1Index;
    }

    if(adc0Index >= SENSOR_PIN_AMT_ADC0 && adc1Index >= SENSOR_PIN_AMT_ADC1){ // Do here so we don't start a read for an invalid pin
        adc0Index = 0;
        adc1Index = 0;
    } else if(adc0Index < SENSOR_PIN_AMT_ADC0){ // Do here so we don't start a read for an invalid pin
        adc->adc0->enableInterrupts(ADCConversionCompleteCallback, adc0Index); // Priority gets lower as it's further in the index array
        adc->adc0->startSingleRead(adc0Pins[adc0Index]); // in callbacks.h
        return;
    } else if(adc1Index < SENSOR_PIN_AMT_ADC1){ // Do here so we don't start a read for an invalid pin
        adc->adc1->enableInterrupts(ADCConversionCompleteCallback, adc1Index); // Priority gets lower as it's further in the index array
        adc->adc1->startSingleRead(adc1Pins[adc1Index]); // in callbacks.h
        return;
    }
    interrupts();

    // Simulate ADC readings to test
    static uint32_t ticks = 0;
    static uint8_t direction = 1;
    // adc0Reads[APPS_1_INDEX] = (1 * (ticks/10)) % 2047; // Simulated value for APPS 1
    // adc0Reads[APPS_2_INDEX] = 1024; // Simulated value for APPS 2

    adc0Reads[APPS_1_INDEX] = 1024 + (1024 * direction); // Simulated value for APPS 1
    adc0Reads[APPS_2_INDEX] = 1024; // Simulated value for APPS 2

    ticks++;
    if (ticks % 100 == 0) {
        direction *= -1;
    }

    // Update each sensors data
    APPS_UpdateData(adc0Reads[APPS_1_INDEX], adc0Reads[APPS_2_INDEX]);
    BSE_UpdateData(adc0Reads[BSE_1_INDEX], adc0Reads[BSE_2_INDEX]);

    // Handle any faults that were raised
    Faults_HandleFaults();

    Motor_UpdateMotor();

    // Update telemetry data
    TelemetryData telemetryData = {
        .APPS_Travel = APPS_GetAPPSReading(),
        .BSEFront_PSI = BSE_GetBSEReading()->bseFront_PSI,
        .BSERear_PSI = BSE_GetBSEReading()->bseRear_PSI,
        .motorState = Motor_GetState(),
    };

    Telemetry_UpdateData(&telemetryData);
}

