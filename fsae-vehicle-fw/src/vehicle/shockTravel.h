#pragma once
#include <cstdint>

typedef struct {
    float LinPot1Voltage; // Shcok Travel 1 Voltage
    float LinPot2Voltage; // Shcok Travel 2 Voltage
    float LinPot3Voltage; // Shcok Travel 3 Voltage
    float LinPot4Voltage; // Shcok Travel 4 Voltage

    float shockTravel1_mm;
    float shockTravel2_mm;
    float shockTravel3_mm;
    float shockTravel4_mm;

    float Shock1RawReading;
    float Shock2RawReading;
    float Shock3RawReading;
    float Shock4RawReading;
} LinpotData;

void Shock_Init();

void ShockTravelUpdateData(uint16_t rawReading1, uint16_t rawReading2,
                           uint16_t rawReading3, uint16_t rawReading4);

LinpotData *Linpot_GetData();
