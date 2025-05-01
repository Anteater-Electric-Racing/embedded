// Anteater Electric Racing, 2025

#include <cmath>

#include "apps.h"

#include "faults.h"

typedef struct {
    float apps1RawReading;
    float apps2RawReading;
} APPSRawData;

static APPSRawData appsRaw;
static APPSData appsData;
static float appsAlpha;

static void checkAndHandleAPPSFault();
static void checkAndHandlePlausibilityFault();

void APPS_Init() {
    appsData.appsReading1_Percentage = 0;
    appsData.appsReading2_Percentage = 0;

    appsRaw.apps1RawReading = 0;
    appsRaw.apps2RawReading = 0;

    appsAlpha = COMPUTE_ALPHA(1000.0F);
}

void checkAndHandleAPPSFault() {
    float difference = fabs(appsData.appsReading1_Percentage - appsData.appsReading2_Percentage);

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
    float newReading1 = rawReading1;
    float newReading2 = rawReading2;

    LOWPASS_FILTER(newReading1, appsRaw.apps1RawReading, appsAlpha);
    LOWPASS_FILTER(rawReading2, appsRaw.apps2RawReading, appsAlpha);

    appsData.appsReading1_Percentage = APPS_3V3_PERCENTAGE(APPS_ADC_TO_VOLTAGE(appsRaw.apps1RawReading));
    appsData.appsReading2_Percentage = APPS_5V_PERCENTAGE(APPS_ADC_TO_VOLTAGE(appsRaw.apps2RawReading));

    checkAndHandleAPPSFault();
    checkAndHandlePlausibilityFault();
}

float APPS_GetAPPSReading() {
    return appsData.appsReading1_Percentage;
    // return (appsData.appsReading1_Percentage + appsData.appsReading2_Percentage) / 2;
}

APPSData APPS_GetAPPSReading_Separate() {
    return appsData;
}