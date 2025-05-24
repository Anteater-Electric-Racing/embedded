// Anteater Electric Racing, 2025

#include <cmath>
#include "utils/utils.h"
#include "apps.h"
#include "vehicle/faults.h"
#include "vehicle/telemetry.h"

typedef struct {
    float appsReading1_Percentage; // Percentage of pedal travel (0 to 1)
    float appsReading2_Percentage; // Percentage of pedal travel (0 to 1)

    float appsReading1_Voltage;    // Voltage reading from the pedal (0 to 3.3V)
    float appsReading2_Voltage;    // Voltage reading from the pedal (0 to 3.3V)

    float apps1RawReading;
    float apps2RawReading;
} APPSData;

static APPSData appsData;
static float appsAlpha;

static void checkAndHandleAPPSFault();
static void checkAndHandlePlausibilityFault();

void APPS_Init() {
    appsData.appsReading1_Percentage = 0;
    appsData.appsReading2_Percentage = 0;

    appsData.appsReading1_Voltage = 0;
    appsData.appsReading2_Voltage = 0;

    appsData.apps1RawReading = 0;
    appsData.apps2RawReading = 0;

    appsAlpha = COMPUTE_ALPHA(100.0F);
}

void APPS_UpdateData(uint32_t rawReading1, uint32_t rawReading2) {
    // Filter incoming values
    LOWPASS_FILTER(rawReading1, appsData.apps1RawReading, appsAlpha);
    LOWPASS_FILTER(rawReading2, appsData.apps2RawReading, appsAlpha);

    appsData.apps1RawReading = rawReading1;
    appsData.apps2RawReading = rawReading2;

    // Convert ADC values to voltage
    appsData.appsReading1_Voltage = ADC_VALUE_TO_VOLTAGE(appsData.apps1RawReading);
    appsData.appsReading2_Voltage = ADC_VALUE_TO_VOLTAGE(appsData.apps2RawReading);

    // Map voltage to percentage of throttle travel, limiting to 0-1 range
    appsData.appsReading1_Percentage = LINEAR_MAP(appsData.appsReading1_Voltage, APPS_3V3_MIN, APPS_3V3_MAX, 0.0F, 1.0F);
    appsData.appsReading2_Percentage = LINEAR_MAP(appsData.appsReading2_Voltage, APPS_5V_MIN, APPS_5V_MAX, 0.0F, 1.0F);

    if(appsData.appsReading1_Percentage < 0.0F) {
        appsData.appsReading1_Percentage = 0.0F;
    } else if(appsData.appsReading1_Percentage > 1.0F) {
        appsData.appsReading1_Percentage = 1.0F;
    }

    if(appsData.appsReading2_Percentage < 0.0F) {
        appsData.appsReading2_Percentage = 0.0F;
    } else if(appsData.appsReading2_Percentage > 1.0F) {
        appsData.appsReading2_Percentage = 1.0F;
    }

    checkAndHandleAPPSFault();
    checkAndHandlePlausibilityFault();
    // Faults_HandleFaults();
}

float APPS_GetAPPSReading() {
    return (appsData.appsReading1_Percentage + appsData.appsReading2_Percentage) / 2;
}

float APPS_GetAPPSReading1() {
    return appsData.appsReading1_Percentage;
}

float APPS_GetAPPSReading2() {
    return appsData.appsReading2_Percentage;
}

static void checkAndHandleAPPSFault() {
    // Check for open/short circuit
    float difference = abs(appsData.appsReading1_Percentage - appsData.appsReading2_Percentage);
    Serial.print("Difference is: ");
    Serial.println(difference);
    Serial.print("Percent APPS1: ");
    Serial.println(appsData.appsReading1_Percentage);
    Serial.print("Percent APPS2: ");
    Serial.println(appsData.appsReading2_Percentage);
    if(appsData.appsReading1_Voltage < APPS_3V3_FAULT_MIN ||
       appsData.appsReading1_Voltage > APPS_3V3_FAULT_MAX ||
       appsData.appsReading2_Voltage < APPS_5V_FAULT_MIN ||
       appsData.appsReading2_Voltage > APPS_5V_FAULT_MAX) {
        Serial.println("Setting APPS fault");
        Faults_SetFault(FAULT_APPS);
        return;
    }
    if (difference > APPS_IMPLAUSABILITY_THRESHOLD) {
        Faults_SetFault(FAULT_APPS);
        return;
    } else {
        Serial.println("Clearing fault in handle");
        Faults_ClearFault(FAULT_APPS);
    }
}

static void checkAndHandlePlausibilityFault() {
    float BSEReading_Front = BSE_GetBSEReading()->bseFront_PSI;
    float BSEReading_Rear = BSE_GetBSEReading()->bseRear_PSI;

    float BSEReading = BSEReading_Front;
    if (BSEReading_Rear > BSEReading_Front)
        BSEReading = BSEReading_Rear;

    Serial.print("BSE Reading: ");
    Serial.println(BSEReading);

    if (APPS_GetAPPSReading() > APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD &&
        BSEReading > APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD) {
        Faults_SetFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    } else {
        if (APPS_GetAPPSReading() < APPS_BSE_PLAUSIBILITY_RESET_THRESHOLD){
            Faults_ClearFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
        }
    }
}
