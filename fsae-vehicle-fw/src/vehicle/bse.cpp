// Anteater Electric Racing, 2025

#define BSE_LOWER_THRESHOLD 0.5
#define BSE_UPPER_THRESHOLD 4.5
#define BSE_IMPLAUSABILITY_THRESHOLD 0.1

#include <cmath>

#include "bse.h"

#include "vehicle/faults.h"

static void checkAndHandleBSEFault();
static void verifySensorAgreement();

void BSE_Init() {
    bseData.bseReading1 = 0;
    bseData.bseReading2 = 0;
}

void checkAndHandleBSEFault() {
    if (bseData.bseReading1 < BSE_LOWER_THRESHOLD ||
        bseData.bseReading1 > BSE_UPPER_THRESHOLD) {
        Faults_SetFault(FAULT_BSE);
    } else {
        Faults_ClearFault(FAULT_BSE);
    }
}

void verifySensorAgreement() {
    // should notify telemetry if brakes have a disagreement
    if (abs(bseData.bseReading1 - bseData.bseReading2) >
        BSE_IMPLAUSABILITY_THRESHOLD) {
        // set fault code
    } else {
        // clear fault code
    }
}

void BSE_UpdateData(BSEData *data) {
    bseData.bseReading1 = data->bseReading1;
    bseData.bseReading2 = data->bseReading2;

    checkAndHandleBSEFault();
    verifySensorAgreement();
}

float BSE_GetBSEReading() {
    // regen braking
    return (bseData.bseReading1 + bseData.bseReading2) / 2;
}
