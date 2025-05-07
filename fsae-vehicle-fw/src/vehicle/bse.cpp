// Anteater Electric Racing, 2025

#include <cmath>

#include "utils/utils.h"
#include "bse.h"

#include "faults.h"

#define ADC_RESOLUTION 12
#define ADC_MAX_VALUE ((1 << ADC_RESOLUTION) - 1)

#define BSE_VOLTAGE_DIVIDER 2.0F // TODO: Update with real value
#define BSE_ADC_VALUE_TO_VOLTAGE(x) (x * (LOGIC_LEVEL_V / ADC_MAX_VALUE)) * BSE_VOLTAGE_DIVIDER // ADC value to voltage conversion

#define BSE_SENSOR_VMIN 0.5F
#define BSE_SENSOR_VMAX 4.5F
#define BSE_SENSOR_PMIN 0.0F
#define BSE_SENSOR_PMAX 100.0F
#define BSE_VOLTAGE_TO_PSI(x) (((x) - BSE_SENSOR_VMIN) * ((BSE_SENSOR_PMAX - BSE_SENSOR_PMIN) / (BSE_SENSOR_VMAX - BSE_SENSOR_VMIN)) + BSE_SENSOR_PMIN) // Voltage to PSI conversion

#define BSE_LOWER_THRESHOLD 0.5F
#define BSE_UPPER_THRESHOLD 4.5F
#define BSE_IMPLAUSABILITY_THRESHOLD 0.1F

#define BSE_CUTOFF_HZ 100.0F

typedef struct{
    float bseRawFront;
    float bseRawRear;
} BSERawData;

static BSEData bseData;
static BSERawData bseRawData;
static float bseAlpha;

void BSE_Init() {
    bseData.bseFront_PSI = 0;
    bseData.bseRear_PSI = 0;

    bseRawData.bseRawFront = 0;
    bseRawData.bseRawRear = 0;

    bseAlpha = COMPUTE_ALPHA(BSE_CUTOFF_HZ); // 10Hz cutoff frequency, 0.01s sample time
}

void BSE_UpdateData(uint32_t bseReading1, uint32_t bseReading2){
    // Filter incoming values
    LOWPASS_FILTER(bseReading1, bseRawData.bseRawFront, bseAlpha);
    LOWPASS_FILTER(bseReading2, bseRawData.bseRawRear, bseAlpha);

    float bseVoltage1 = ADC_VALUE_TO_VOLTAGE(bseRawData.bseRawFront);
    float bseVoltage2 = ADC_VALUE_TO_VOLTAGE(bseRawData.bseRawRear);

    // Check BSE open/short circuit
    if(bseVoltage1 < BSE_LOWER_THRESHOLD ||
       bseVoltage1 > BSE_UPPER_THRESHOLD ||
       bseVoltage2 < BSE_LOWER_THRESHOLD ||
       bseVoltage2 > BSE_UPPER_THRESHOLD) {
        // Faults_SetFault(FAULT_BSE);
    } else {
        Faults_ClearFault(FAULT_BSE);
    }

    bseData.bseFront_PSI = BSE_VOLTAGE_TO_PSI(bseVoltage1);
    bseData.bseRear_PSI = BSE_VOLTAGE_TO_PSI(bseVoltage2);
}

BSEData* BSE_GetBSEReading() {
    return &bseData;
}
