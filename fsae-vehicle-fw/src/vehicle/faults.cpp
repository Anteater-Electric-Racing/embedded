// Anteater Electric Racing, 2025


#define FAULT_OVER_CURRENT_MASK (0x1)
#define FAULT_UNDER_VOLTAGE_MASK (0x1 << 1)
#define FAULT_OVER_TEMP_MASK (0x1 << 2)
#define FAULT_APPS_MASK (0x1 << 3)
#define FAULT_BSE_MASK (0x1 << 4)
#define FAULT_BPPS_MASK (0x1 << 5)
#define FAULT_APPS_BRAKE_PLAUSIBILITY_MASK (0x1 << 6)

//Inverter Faults
#define FAULT_DC_OVER_VOLT_FAULT_MASK (0x1 << 7)
#define FAULT_MOTOR_PHASE_CURR_FAULT_MASK (0x1 << 8)
#define FAULT_MCU_OVER_HOT_FAULT_MASK (0x1 << 9)
#define FAULT_RESOLVER_FAULT_MASK (0x1 << 10)
#define FAULT_PHASE_CURR_SENSOR_FAULT_MASK (0x1 << 11)
#define FAULT_MOTOR_OVER_SPD_FAULT_MASK (0x1 << 12)
#define FAULT_DRV_MOTOR_OVER_HOT_FAULT_MASK (0x1 << 13)
#define FAULT_DC_MAIN_WIRE_OVER_CURR_FAULT_MASK (0x1 << 14)
#define FAULT_DRV_MOTOR_OVER_COOL_FAULT_MASK (0x1 << 15)
#define FAULT_DC_LOW_VOLT_WARNING_MASK (0x1 << 16)
#define FAULT_MCU_12V_LOW_VOLT_WARNING_MASK (0x1 << 17)
#define FAULT_MOTOR_STALL_FAULT_MASK (0x1 << 18)
#define FAULT_MOTOR_OPEN_PHASE_FAULT_MASK (0x1 << 19)



#include "vehicle/faults.h"
#include "utils/utils.h"
#include "vehicle/motor.h"

#if DEBUG_FLAG
#include <Arduino.h>
#endif

static uint32_t faultBitMap;

void Faults_Init() { faultBitMap = 0; }

void Faults_SetFault(FaultType fault) {
    switch (fault) {
    case FAULT_NONE: {
        break;
    }
    case FAULT_OVER_CURRENT: {
        faultBitMap |= FAULT_OVER_CURRENT_MASK;
        break;
    }
    case FAULT_UNDER_VOLTAGE: {
        faultBitMap |= FAULT_UNDER_VOLTAGE_MASK;
        break;
    }
    case FAULT_OVER_TEMP: {
        faultBitMap |= FAULT_OVER_TEMP_MASK;
        break;
    }
    case FAULT_APPS: {
#if DEBUG_FLAG
        Serial.println("Setting APPS fault");
#endif
        faultBitMap |= FAULT_APPS_MASK;
        break;
    }
    case FAULT_BSE: {
#if DEBUG_FLAG
        Serial.println("Setting BSE fault");
#endif
        faultBitMap |= FAULT_BSE_MASK;
        break;
    }
    case FAULT_BPPS: {
        faultBitMap |= FAULT_BPPS_MASK;
        break;
    }
    case FAULT_APPS_BRAKE_PLAUSIBILITY: {
#if DEBUG_FLAG
        Serial.println("Setting APPS Plausibility fault");
#endif
        faultBitMap |= FAULT_APPS_BRAKE_PLAUSIBILITY_MASK;
        break;
    }

    // Inverter Faults
    case FAULT_DC_OVER_VOLT: {
        faultBitMap |= FAULT_DC_OVER_VOLT_FAULT_MASK;
        break;
    }
    case FAULT_MOTOR_PHASE_CURR: {
        faultBitMap |= FAULT_MOTOR_PHASE_CURR_FAULT_MASK;
        break;
    }
    case FAULT_OVER_HOT: {
        faultBitMap |= FAULT_MCU_OVER_HOT_FAULT_MASK;
        break;
    }
    case FAULT_RESOLVER: {
        faultBitMap |= FAULT_RESOLVER_FAULT_MASK;
        break;
    }
    case FAULT_PHASE_CURR_SENSOR: {
        faultBitMap |= FAULT_PHASE_CURR_SENSOR_FAULT_MASK;
        break;
    }
    case FAULT_MOTOR_OVER_SPEED: {
        faultBitMap |= FAULT_MOTOR_OVER_SPD_FAULT_MASK;
        break;
    }
    case FAULT_DRV_OVER_HOT: {
        faultBitMap |= FAULT_DRV_MOTOR_OVER_HOT_FAULT_MASK;
        break;
    }
    case FAULT_DC_OVER_CURR: {
        faultBitMap |= FAULT_DC_MAIN_WIRE_OVER_CURR_FAULT_MASK;
        break;
    }
    case FAULT_DRV_OVER_COOL: {
        faultBitMap |= FAULT_DRV_MOTOR_OVER_COOL_FAULT_MASK;
        break;
    }
    case FAULT_DC_LOW_VOLT_WARN: {
        faultBitMap |= FAULT_DC_LOW_VOLT_WARNING_MASK;
        break;
    }
    case FAULT_12V_LOW_VOLT_WARN: {
        faultBitMap |= FAULT_MCU_12V_LOW_VOLT_WARNING_MASK;
        break;
    }
    case FAULT_MOTOR_STALL: {
        faultBitMap |= FAULT_MOTOR_STALL_FAULT_MASK;
        break;
    }
    case FAULT_MOTOR_OPEN_PHASE: {
        faultBitMap |= FAULT_MOTOR_OPEN_PHASE_FAULT_MASK;
        break;
    }

    default: {
        break;
    }
    }
}
void Faults_ClearFault(FaultType fault) {
    switch (fault) {
    case FAULT_NONE: {
        break;
    }
    case FAULT_OVER_CURRENT: {
        faultBitMap &= ~FAULT_OVER_CURRENT_MASK;
        break;
    }
    case FAULT_UNDER_VOLTAGE: {
        faultBitMap &= ~FAULT_UNDER_VOLTAGE_MASK;
        break;
    }
    case FAULT_OVER_TEMP: {
        faultBitMap &= ~FAULT_OVER_TEMP_MASK;
        break;
    }
    case FAULT_APPS: {
#if DEBUG_FLAG
        Serial.println("Clearing APPS fault");
#endif
        faultBitMap &= ~FAULT_APPS_MASK;
        break;
    }
    case FAULT_BSE: {
        faultBitMap &= ~FAULT_BSE_MASK;
        break;
    }
    case FAULT_BPPS: {
        faultBitMap &= ~FAULT_BPPS_MASK;
        break;
    }
    case FAULT_APPS_BRAKE_PLAUSIBILITY: {
#if DEBUG_FLAG
        Serial.println("Clearing APPS BSE plausibility fault");
#endif
        faultBitMap &= ~FAULT_APPS_BRAKE_PLAUSIBILITY_MASK;
        break;
    }

    // Inverter Faults
    case FAULT_DC_OVER_VOLT: {
        faultBitMap &= ~FAULT_DC_OVER_VOLT_FAULT_MASK;
        break;
    }
    case FAULT_MOTOR_PHASE_CURR: {
        faultBitMap &= ~FAULT_MOTOR_PHASE_CURR_FAULT_MASK;
        break;
    }
    case FAULT_OVER_HOT: {
        faultBitMap &= ~FAULT_MCU_OVER_HOT_FAULT_MASK;
        break;
    }
    case FAULT_RESOLVER: {
        faultBitMap &= ~FAULT_RESOLVER_FAULT_MASK;
        break;
    }
    case FAULT_PHASE_CURR_SENSOR: {
        faultBitMap &= ~FAULT_PHASE_CURR_SENSOR_FAULT_MASK;
        break;
    }
    case FAULT_MOTOR_OVER_SPEED: {
        faultBitMap &= ~FAULT_MOTOR_OVER_SPD_FAULT_MASK;
        break;
    }
    case FAULT_DRV_OVER_HOT: {
        faultBitMap &= ~FAULT_DRV_MOTOR_OVER_HOT_FAULT_MASK;
        break;
    }
    case FAULT_DC_OVER_CURR: {
        faultBitMap &= ~FAULT_DC_MAIN_WIRE_OVER_CURR_FAULT_MASK;
        break;
    }
    case FAULT_DRV_OVER_COOL: {
        faultBitMap &= ~FAULT_DRV_MOTOR_OVER_COOL_FAULT_MASK;
        break;
    }
    case FAULT_DC_LOW_VOLT_WARN: {
        faultBitMap &= ~FAULT_DC_LOW_VOLT_WARNING_MASK;
        break;
    }
    case FAULT_12V_LOW_VOLT_WARN: {
        faultBitMap &= ~FAULT_MCU_12V_LOW_VOLT_WARNING_MASK;
        break;
    }
    case FAULT_MOTOR_STALL: {
        faultBitMap &= ~FAULT_MOTOR_STALL_FAULT_MASK;
        break;
    }
    case FAULT_MOTOR_OPEN_PHASE: {
        faultBitMap &= ~FAULT_MOTOR_OPEN_PHASE_FAULT_MASK;
        break;
    }

    default: {
        break;
    }
    }
}

// currently having all faults being handled the same but leaving room for
// future customization
void Faults_HandleFaults() {
#if DEBUG_FLAG
    Serial.print("Fault bitmap: ");
    Serial.println(faultBitMap);
#endif

    if (faultBitMap == 0) {
#if DEBUG_FLAG
        Serial.println("Clearing all faults in handle faults");
#endif
        Motor_ClearFaultState();
        return;
    }


    if (faultBitMap & FAULT_OVER_CURRENT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_UNDER_VOLTAGE_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_OVER_TEMP_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_APPS_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_BSE_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_BPPS_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_APPS_BRAKE_PLAUSIBILITY_MASK) {
        Motor_SetFaultState();
    }

    // Inverter Faults
    if (faultBitMap & FAULT_DC_OVER_VOLT_FAULT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_MOTOR_PHASE_CURR_FAULT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_MCU_OVER_HOT_FAULT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_RESOLVER_FAULT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_PHASE_CURR_SENSOR_FAULT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_MOTOR_OVER_SPD_FAULT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_DRV_MOTOR_OVER_HOT_FAULT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_DC_MAIN_WIRE_OVER_CURR_FAULT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_DRV_MOTOR_OVER_COOL_FAULT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_DC_LOW_VOLT_WARNING_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_MCU_12V_LOW_VOLT_WARNING_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_MOTOR_STALL_FAULT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_MOTOR_OPEN_PHASE_FAULT_MASK) {
        Motor_SetFaultState();
    }
}

// for telemetry
uint32_t Faults_GetFaults() { return faultBitMap; }

bool Faults_CheckAllClear() { return faultBitMap == 0; }
