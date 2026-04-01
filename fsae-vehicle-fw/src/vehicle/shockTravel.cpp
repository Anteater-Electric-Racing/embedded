/* AER 2026 */

#include "shockTravel.h"
#include "../utils/utils.h"
#include "vehicle/telemetry.h"
#include <arduino_freertos.h>
#include <cmath>

template <typename T> T constrain(T val, T minVal, T maxVal) {
    if (val < minVal)
        return minVal;
    if (val > maxVal)
        return maxVal;
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
void ShockTravelUpdateData(uint16_t rawReading1, uint16_t rawReading2,
                           uint16_t rawReading3, uint16_t rawReading4) {
    linPots.Shock1RawReading = abs(4095 - rawReading1);
    linPots.Shock2RawReading = abs(4095 - rawReading2);
    linPots.Shock3RawReading = abs(4095 - rawReading3);
    linPots.Shock4RawReading = abs(4095 - rawReading4);

    // Serial.println("\n\nRAW:");
    // Serial.println(linPots.Shock1RawReading);
    // Serial.println(linPots.Shock2RawReading);
    // Serial.println(linPots.Shock3RawReading);
    // Serial.println(linPots.Shock4RawReading);

    linPots.LinPot1Voltage =
        abs(ADC_VALUE_TO_VOLTAGE(linPots.Shock1RawReading));
    linPots.LinPot2Voltage =
        abs(ADC_VALUE_TO_VOLTAGE(linPots.Shock2RawReading));
    linPots.LinPot3Voltage =
        abs(ADC_VALUE_TO_VOLTAGE(linPots.Shock3RawReading));
    linPots.LinPot4Voltage =
        abs(ADC_VALUE_TO_VOLTAGE(linPots.Shock4RawReading));

    // Serial.println("\n\nVOLTAGE:");
    // Serial.println(linPots.LinPot1Voltage);
    // Serial.println(linPots.LinPot2Voltage);
    // Serial.println(linPots.LinPot3Voltage);
    // Serial.println(linPots.LinPot4Voltage);

    linPots.shockTravel1_mm =
        constrain((linPots.LinPot1Voltage / 0.14f) * SHOCK_TRAVEL_MAX_MM, 0.0f,
                  SHOCK_TRAVEL_MAX_MM);
    linPots.shockTravel2_mm =
        constrain((linPots.LinPot2Voltage / 0.14f) * SHOCK_TRAVEL_MAX_MM, 0.0f,
                  SHOCK_TRAVEL_MAX_MM);
    linPots.shockTravel3_mm =
        constrain((linPots.LinPot3Voltage / 0.14f) * SHOCK_TRAVEL_MAX_MM, 0.0f,
                  SHOCK_TRAVEL_MAX_MM);
    linPots.shockTravel4_mm =
        constrain((linPots.LinPot4Voltage / 0.14f) * SHOCK_TRAVEL_MAX_MM, 0.0f,
                  SHOCK_TRAVEL_MAX_MM);

    // Serial.println("\n\nShockTravelmm:");
    // Serial.println(linPots.shockTravel1_mm);
    // //Serial.println(linPots.shockTravel3_mm);
    // Serial.println(linPots.shockTravel4_mm);
    // Serial.println(linPots.shockTravel2_mm);
}

LinpotData *Linpot_GetData() { return &linPots; }
