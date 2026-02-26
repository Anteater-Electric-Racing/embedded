#include <cmath>
#include "shockTravel.h"
#include "vehicle/telemetry.h"
#include <arduino_freertos.h>
#include "../utils/utils.h"
typedef struct {
    float LinPot1Voltage; //Shcok Travel 1 Voltage
    float LinPot2Voltage; //Shcok Travel 2 Voltage
    float LinPot3Voltage; //Shcok Travel 3 Voltage
    float LinPot4Voltage; //Shcok Travel 4 Voltage

    float shockTravel1_mm;
    float shockTravel2_mm;
    float shockTravel3_mm;
    float shockTravel4_mm;

    float Shock1RawReading;
    float Shock2RawReading;
    float Shock3RawReading;
    float Shock4RawReading;
} LinpotData;

template <typename T>
T constrain(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

static LinpotData linPots;
void Shock_Init() {
    linPots.LinPot1Voltage = 0;
    linPots.LinPot2Voltage = 0;
    linPots.LinPot3Voltage = 0;
    linPots.LinPot4Voltage = 0;

    linPots.shockTravel1_mm = 0;
    linPots.shockTravel2_mm = 0;
    linPots.shockTravel3_mm = 0;
    linPots.shockTravel4_mm = 0;

    linPots.Shock1RawReading = 0;
    linPots.Shock2RawReading = 0;
    linPots.Shock3RawReading = 0;
    linPots.Shock4RawReading = 0;
}
void ShockTravelUpdateData(uint32_t rawReading1, uint32_t rawReading2,uint32_t rawReading3, uint32_t rawReading4) {
    linPots.LinPot1Voltage = ADC_VALUE_TO_VOLTAGE(rawReading1);
    linPots.LinPot2Voltage = ADC_VALUE_TO_VOLTAGE(rawReading2);
    linPots.LinPot3Voltage = ADC_VALUE_TO_VOLTAGE(rawReading3);
    linPots.LinPot4Voltage = ADC_VALUE_TO_VOLTAGE(rawReading4);

    linPots.shockTravel1_mm = constrain((linPots.LinPot1Voltage / 5.0f) * SHOCK_TRAVEL_MAX_MM, 0.0f, SHOCK_TRAVEL_MAX_MM);
    linPots.shockTravel2_mm = constrain((linPots.LinPot2Voltage / 5.0f) * SHOCK_TRAVEL_MAX_MM, 0.0f, SHOCK_TRAVEL_MAX_MM);
    linPots.shockTravel3_mm = constrain((linPots.LinPot3Voltage / 5.0f) * SHOCK_TRAVEL_MAX_MM, 0.0f, SHOCK_TRAVEL_MAX_MM);
    linPots.shockTravel4_mm = constrain((linPots.LinPot4Voltage / 5.0f) * SHOCK_TRAVEL_MAX_MM, 0.0f, SHOCK_TRAVEL_MAX_MM);

}
