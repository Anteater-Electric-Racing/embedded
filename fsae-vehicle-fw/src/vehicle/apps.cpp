// Anteater Electric Racing, 2025

#include <cmath>

#include "utils/utils.h"

#include "apps.h"

#include "vehicle/faults.h"

#define APPS_IMPLAUSABILITY_THRESHOLD 0.1            // 10%
#define APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD 0.25 // 25%
#define APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD 200    // PSI

#define VOLTAGE_DIVIDER 2.0F
#define APPS_ADC_TO_VOLTAGE(x) ((x) * (LOGIC_LEVEL_V / 4095.0F)) * VOLTAGE_DIVIDER

#define APPS_3V3_PERCENTAGE(x) ((x) / 3.3F)
#define APPS_5V_PERCENTAGE(x) ((x) / 5.0F)

typedef struct {
    float apps1RawReading;
    float apps2RawReading;
} APPSRawData;

static APPSRawData appsRaw;
static APPSData appsData;

static void checkAndHandleAPPSFault();
static void checkAndHandlePlausibilityFault();

void APPS_Init() {
    appsData.appsReading1_Percentage = 0;
    appsData.appsReading2_Percentage = 0;

    appsRaw.apps1RawReading = 0;
    appsRaw.apps2RawReading = 0;
}

void checkAndHandleAPPSFault() {
    float difference = abs(appsData.appsReading1_Percentage - appsData.appsReading2_Percentage);

    if (difference > APPS_IMPLAUSABILITY_THRESHOLD) {
        Faults_SetFault(FAULT_APPS);
    } else {
        Faults_ClearFault(FAULT_APPS);
    }
}

void checkAndHandlePlausibilityFault() {
    float BSEReading_Front = BSE_GetBSEReading()->bseFront_PSI;
    float BSEReading_Rear = BSE_GetBSEReading()->bseRear_PSI;

    float BSEReading = BSEReading_Front;
    if (BSEReading_Rear > BSEReading_Front)
        BSEReading = BSEReading_Rear;

    if (APPS_GetAPPSReading() > APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD &&
        BSEReading > APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD) {
        Faults_SetFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    } else {
        Faults_ClearFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    }
}

void APPS_UpdateData(uint32_t rawReading1, uint32_t rawReading2) {
    // Filter incoming values
    LOWPASS_FILTER(rawReading1, appsRaw.apps1RawReading, COMPUTE_ALPHA(100.0F));
    LOWPASS_FILTER(rawReading2, appsRaw.apps2RawReading, COMPUTE_ALPHA(100.0F));

    appsData.appsReading1_Percentage =
        APPS_3V3_PERCENTAGE(APPS_ADC_TO_VOLTAGE(rawReading1));
    appsData.appsReading2_Percentage =
        APPS_5V_PERCENTAGE(APPS_ADC_TO_VOLTAGE(rawReading2));

    checkAndHandleAPPSFault();
    checkAndHandlePlausibilityFault();
}

float APPS_GetAPPSReading() {
    return (appsData.appsReading1_Percentage + appsData.appsReading2_Percentage) / 2;
}
