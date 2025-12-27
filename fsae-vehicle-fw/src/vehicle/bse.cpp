// Anteater Electric Racing, 2025

#include <cmath>

#include "utils/utils.h"

#include "bse.h"

#include "vehicle/faults.h"
#include <arduino_freertos.h>

typedef struct {
    float bseRawFront;
    float bseRawRear;
} BSERawData;

static BSEData bseData;
static BSERawData bseRawData;
static float bseAlpha;
static TickType_t bseLatestHealthyStateTime =
    0; // Set to 0 when fault not detected

void BSE_Init() {
    bseData.bseFront_PSI = 0;
    bseData.bseRear_PSI = 0;

    bseRawData.bseRawFront = 0;
    bseRawData.bseRawRear = 0;

    bseAlpha = COMPUTE_ALPHA(
        BSE_CUTOFF_HZ); // 10Hz cutoff frequency, 0.01s sample time
}

void BSE_UpdateData(uint32_t bseReading1, uint32_t bseReading2) {
    // Filter incoming values
    LOWPASS_FILTER(bseReading1, bseRawData.bseRawFront, bseAlpha);
    LOWPASS_FILTER(bseReading2, bseRawData.bseRawRear, bseAlpha);

    float bseVoltage1 = ADC_VALUE_TO_VOLTAGE(bseRawData.bseRawFront);
    float bseVoltage2 = ADC_VALUE_TO_VOLTAGE(bseRawData.bseRawRear);

#if DEBUG_FLAG
    Serial.print("BSE Voltage: ");
    Serial.println(bseVoltage1);
#endif

    // Check BSE open/short circuit
    if (bseVoltage1 < BSE_LOWER_THRESHOLD ||
        bseVoltage1 > BSE_UPPER_THRESHOLD ||
        bseVoltage2 < BSE_LOWER_THRESHOLD ||
        bseVoltage2 > BSE_UPPER_THRESHOLD) {
        TickType_t now = xTaskGetTickCount();
        TickType_t elapsedTicks = now - bseLatestHealthyStateTime;
        TickType_t elapsedMs = elapsedTicks * portTICK_PERIOD_MS;
        if (elapsedMs > BSE_FAULT_TIME_THRESHOLD_MS) {
#if DEBUG_FLAG
            Serial.println("Setting BSE fault");
#endif
            Faults_SetFault(FAULT_BSE);
        }

    } else {
        bseLatestHealthyStateTime = xTaskGetTickCount();
        Faults_ClearFault(FAULT_BSE);
    }

    bseData.bseFront_PSI = BSE_VOLTAGE_TO_PSI(bseVoltage1);
    bseData.bseRear_PSI = BSE_VOLTAGE_TO_PSI(bseVoltage2);
}

BSEData *BSE_GetBSEReading() { return &bseData; }
