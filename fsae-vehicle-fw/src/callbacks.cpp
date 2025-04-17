// Anteater Electric Racing, 2025

#include <ADC.h>
#include "callbacks.h"
#include "peripherals/adc.h"
#include "vehicle/telemetry.h"

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

        // TODO: transmit data
        Telemetry_UpdateADCData(adc0Reads, adc1Reads);

    } else if(adc0Index < SENSOR_PIN_AMT_ADC0){ // Do here so we don't start a read for an invalid pin
        adc->adc0->enableInterrupts(ADCConversionCompleteCallback, adc0Index); // Priority gets lower as it's further in the index array
        adc->adc0->startSingleRead(adc0Pins[adc0Index]); // in callbacks.h
    } else if(adc1Index < SENSOR_PIN_AMT_ADC1){ // Do here so we don't start a read for an invalid pin
        adc->adc1->enableInterrupts(ADCConversionCompleteCallback, adc1Index); // Priority gets lower as it's further in the index array
        adc->adc1->startSingleRead(adc1Pins[adc1Index]); // in callbacks.h
    }
    interrupts();
}

