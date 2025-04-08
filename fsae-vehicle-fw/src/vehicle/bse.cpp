// Anteater Electric Racing, 2025

#define LOGIC_LEVEL_V 3.3F
#define BSE_VOLTAGE_DIVIDER 2.0F // TODO: Update with real value
#define BSE_ADC_VALUE_TO_VOLTAGE(x) (x * (LOGIC_LEVEL_V / 4095.0F)) * BSE_VOLTAGE_DIVIDER // ADC value to voltage conversion

#define BSE_VOLTAGE_TO_PSI(x) x // Voltage to PSI conversion

#define BSE_LOWER_THRESHOLD 0.5
#define BSE_UPPER_THRESHOLD 4.5
#define BSE_IMPLAUSABILITY_THRESHOLD 0.1

#include <cmath>

#include "bse.h"

#include "vehicle/faults.h"

static BSEData bseData;

void BSE_Init() {
    bseData.bseFront_PSI = 0;
    bseData.bseRear_PSI = 0;
}

void BSE_UpdateData(uint32_t bseReading1, uint32_t bseReading2){
    float bseVoltage1 = BSE_ADC_VALUE_TO_VOLTAGE(bseReading1);
    float bseVoltage2 = BSE_ADC_VALUE_TO_VOLTAGE(bseReading2);

    // Check BSE open/short circuit
    if(bseVoltage1 < BSE_LOWER_THRESHOLD ||
       bseVoltage1 > BSE_UPPER_THRESHOLD ||
       bseVoltage2 < BSE_LOWER_THRESHOLD ||
       bseVoltage2 > BSE_UPPER_THRESHOLD) {
        Faults_SetFault(FAULT_BSE);
    } else {
        Faults_ClearFault(FAULT_BSE);
    }

    bseData.bseFront_PSI = BSE_VOLTAGE_TO_PSI(bseVoltage1);
    bseData.bseRear_PSI = BSE_VOLTAGE_TO_PSI(bseVoltage2);
}

BSEData* BSE_GetBSEReading() {
    return &bseData;
}
