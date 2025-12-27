// Anteater Electric Racing, 2025

#include "apps.h"
#include "utils/utils.h"
#include "vehicle/faults.h"
#include "vehicle/telemetry.h"
#include <arduino_freertos.h>
#include <cmath>

typedef struct {
    float appsReading1_Percentage; // Percentage of pedal travel (0 to 1)
    float appsReading2_Percentage; // Percentage of pedal travel (0 to 1)

    float appsReading1_Voltage; // Voltage reading from the pedal (0 to 3.3V)
    float appsReading2_Voltage; // Voltage reading from the pedal (0 to 5V)

    float apps1RawReading;
    float apps2RawReading;
} APPSData;

static APPSData appsData;
static float appsAlpha;
static TickType_t appsLatestHealthyStateTime =
    0; // Set to 0 when fault not detected

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

void APPS_UpdateData(uint16_t rawReading1,
                     uint16_t rawReading2) { // changed uint16 from 32
    // Serial.print("Raw APPS1: ");
    // Serial.println(rawReading1);
    // Serial.print("Raw APPS2: ");
    // Serial.println(rawReading2);

    if (rawReading1 > APPS1_20PCT_ADC)
        rawReading1 = APPS1_20PCT_ADC;
    if (rawReading2 > APPS2_20PCT_ADC)
        rawReading2 = APPS2_20PCT_ADC;

    LOWPASS_FILTER(rawReading1, appsData.apps1RawReading, appsAlpha);
    LOWPASS_FILTER(rawReading2, appsData.apps2RawReading, appsAlpha);
    Serial.print("\n\n\n\n\n");
    Serial.print("Raw APPS1: ");
    Serial.println(appsData.apps1RawReading);
    Serial.print("Raw APPS2: ");
    Serial.println(appsData.apps2RawReading);

    // after LOWPASS_FILTER
    appsData.appsReading1_Percentage =
        LINEAR_MAP(appsData.apps1RawReading, (float)APPS1_REST_ADC,
                   (float)APPS1_20PCT_ADC, 0.0F, 1.0F);

    appsData.appsReading2_Percentage =
        LINEAR_MAP(appsData.apps2RawReading, (float)APPS2_REST_ADC,
                   (float)APPS2_20PCT_ADC, 0.0F, 1.0F);

    // clamp since LINEAR_MAP doesn't clamp
    appsData.appsReading1_Percentage =
        CLAMP01(appsData.appsReading1_Percentage);
    appsData.appsReading2_Percentage =
        CLAMP01(appsData.appsReading2_Percentage);

    // Convert ADC values to voltage
    appsData.appsReading1_Voltage =
        ADC_VALUE_TO_VOLTAGE(appsData.apps1RawReading);
    appsData.appsReading2_Voltage =
        ADC_VALUE_TO_VOLTAGE(appsData.apps2RawReading);

    Serial.print("APPS1 RAW Voltage: ");
    Serial.println(appsData.appsReading1_Voltage);
    Serial.print("APPS2 RAW Voltage: ");
    Serial.println(appsData.appsReading2_Voltage);

    if (appsData.appsReading1_Voltage < APPS_3V3_MIN) {
        appsData.appsReading1_Voltage = APPS_3V3_MIN;
    } else if (appsData.appsReading1_Voltage > APPS_3V3_MAX) {
        appsData.appsReading1_Voltage = APPS_3V3_MAX;
    }

    if (appsData.appsReading2_Voltage < APPS_5V_MIN) {
        appsData.appsReading2_Voltage = APPS_5V_MIN;
    } else if (appsData.appsReading2_Voltage > APPS_5V_MAX) {
        appsData.appsReading2_Voltage = APPS_5V_MAX;
    }
    // Moved this upwards to before the clamping of percentage

    // Map voltage to percentage of throttle travel, limiting to 0-1 range
    // appsData.appsReading1_Percentage =
    //     LINEAR_MAP(appsData.apps1RawReading, 0.0F, (float)APPS1_20PCT_ADC,
    //     0.0F, 1.0F);

    // appsData.appsReading2_Percentage =
    //     LINEAR_MAP(appsData.apps2RawReading, 0.0F, (float)APPS2_20PCT_ADC,
    //     0.0F, 1.0F);

    // Serial.print("APPS1 raw Perc: ");
    // Serial.println(appsData.appsReading1_Percentage);
    // Serial.print("APPS2 raw Perc: ");
    // Serial.println(appsData.appsReading2_Percentage);

    if (appsData.appsReading1_Percentage < 0.0F) {
        appsData.appsReading1_Percentage = 0.0F;
    } else if (appsData.appsReading1_Percentage > 1.0F) {
        appsData.appsReading1_Percentage = 1.0F;
    }

    if (appsData.appsReading2_Percentage < 0.0F) {
        appsData.appsReading2_Percentage = 0.0F;
    } else if (appsData.appsReading2_Percentage > 1.0F) {
        appsData.appsReading2_Percentage = 1.0F;
    }

    checkAndHandleAPPSFault();
    checkAndHandlePlausibilityFault();
}

float APPS_GetAPPSReading() {
    return (appsData.appsReading1_Percentage +
            appsData.appsReading2_Percentage) /
           2.0;
}

float APPS_GetAPPSReading1() { return appsData.appsReading1_Percentage; }

float APPS_GetAPPSReading2() { return appsData.appsReading2_Percentage; }

static void checkAndHandleAPPSFault() {
    // Check for open/short circuit
    float difference = abs(appsData.appsReading1_Percentage -
                           appsData.appsReading2_Percentage);

    // # if DEBUG_FLAG
    Serial.print("Difference is: ");
    Serial.println(difference);
    Serial.print("Percent APPS1: ");
    Serial.println(appsData.appsReading1_Percentage);
    Serial.print("Percent APPS2: ");
    Serial.println(appsData.appsReading2_Percentage);

    // # endif

    if (appsData.appsReading1_Voltage < APPS_3V3_FAULT_MIN ||
        appsData.appsReading1_Voltage > APPS_3V3_FAULT_MAX ||
        appsData.appsReading2_Voltage < APPS_5V_FAULT_MIN ||
        appsData.appsReading2_Voltage > APPS_5V_FAULT_MAX) {

        TickType_t now = xTaskGetTickCount();
        TickType_t elapsedTicks = now - appsLatestHealthyStateTime;
        TickType_t elapsedMs = elapsedTicks * portTICK_PERIOD_MS;

        if (elapsedMs > APPS_FAULT_TIME_THRESHOLD_MS) {
#if DEBUG_FLAG
            Serial.println("Setting APPS fault");
#endif
            Faults_SetFault(FAULT_APPS);
            return;
        }
    } else {
        appsLatestHealthyStateTime = xTaskGetTickCount();
        Faults_ClearFault(FAULT_APPS);
    }

    if (difference > APPS_IMPLAUSABILITY_THRESHOLD) {
        Faults_SetFault(FAULT_APPS);
        return;
    } else {
#if DEBUG_FLAG
        Serial.println("Clearing fault in handle");
#endif
        Faults_ClearFault(FAULT_APPS);
    }
}

static void checkAndHandlePlausibilityFault() {
    float BSEReading_Front = BSE_GetBSEReading()->bseFront_PSI;
    float BSEReading_Rear = BSE_GetBSEReading()->bseRear_PSI;

    float BSEReading = BSEReading_Front;
    if (BSEReading_Rear > BSEReading_Front) {
        BSEReading = BSEReading_Rear;
    }

#if DEBUG_FLAG
    Serial.print("BSE Reading: ");
    Serial.println(BSEReading);
#endif

    if (APPS_GetAPPSReading() > APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD &&
        BSEReading > APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD) {
        Faults_SetFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    } else {
        if (APPS_GetAPPSReading() < APPS_BSE_PLAUSIBILITY_RESET_THRESHOLD) {
            Faults_ClearFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
        }
    }
}
