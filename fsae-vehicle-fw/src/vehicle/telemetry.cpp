// Anteater Electric Racing, 2025
#define TELEMETRY_CAN_ID 0x666 // Example CAN ID for telemetry messages
#define TELEMETRY_PERIOD_MS 10 // Telemetry update period in milliseconds

#include <arduino_freertos.h>

#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/telemetry.h"

#include "utils/utils.h"

#include "peripherals/can.h"

TelemetryData telemetryData;

void Telemetry_Init() {
    // TODO: Update initialization
    telemetryData = { // Fill with reasonable dummy values
        .APPS_Travel = 0.0F,
        .BSEFront_PSI = 0.0F,
        .BSERear_PSI = 0.0F,
        .accumulatorVoltage = 0.0F,
        .accumulatorTemp_F = 25.0F,
        .motorState = MOTOR_STATE_OFF,
        .motorSpeed = 0.0F,
        .motorTorque = 0.0F,
        .maxMotorTorque = 0.0F,
        .maxMotorBrakeTorque = 0.0F,
        .motorDirection = DIRECTION_STANDBY,
        .mcuMainState = STATE_STANDBY,
        .mcuWorkMode = WORK_MODE_STANDBY,
        .motorTemp = 25,
        .mcuTemp = 25,
        .dcMainWireOverVoltFault = false,
        .motorPhaseCurrFault = false,
        .mcuOverHotFault = false,
        .resolverFault = false,
        .phaseCurrSensorFault = false,
        .motorOverSpdFault = false,
        .drvMotorOverHotFault = false,
        .dcMainWireOverCurrFault = false,
        .drvMotorOverCoolFault = false,
        .mcuMotorSystemState = false,
        .mcuTempSensorState = false,
        .motorTempSensorState = false,
        .dcVoltSensorState = false,
        .dcLowVoltWarning = false,
        .mcu12VLowVoltWarning = false,
        .motorStallFault = false,
        .motorOpenPhaseFault = false,
        .mcuWarningLevel = ERROR_NONE,
        .mcuVoltage = 0.0F,
        .mcuCurrent = 0.0F,
        .motorPhaseCurr = 0.0F,
    };
}

void threadTelemetry(void *pvParameters){
    static TickType_t lastWakeTime = xTaskGetTickCount(); // Initialize the last wake time
    while(true){
        taskENTER_CRITICAL(); // Enter critical section
        telemetryData = {
            .APPS_Travel = APPS_GetAPPSReading(),
            .BSEFront_PSI = BSE_GetBSEReading()->bseFront_PSI,
            .BSERear_PSI = BSE_GetBSEReading()->bseRear_PSI,
            .accumulatorVoltage = 0.0F, // TODO: Replace with actual accumulator voltage reading
            .accumulatorTemp_F = 0.0F, // TODO: Replace with actual accumulator temperature reading
            .motorState = Motor_GetState(),

            .motorSpeed = MCU_GetMCU1Data()->motorSpeed,
            .motorTorque = MCU_GetMCU1Data()->motorTorque,
            .maxMotorTorque = MCU_GetMCU1Data()->maxMotorTorque,
            .maxMotorBrakeTorque = MCU_GetMCU1Data()->maxMotorBrakeTorque,
            .motorDirection = MCU_GetMCU1Data()->motorDirection,
            .mcuMainState = MCU_GetMCU1Data()->mcuMainState,
            .mcuWorkMode = MCU_GetMCU1Data()->mcuWorkMode,

            .motorTemp = MCU_GetMCU2Data()->motorTemp,
            .mcuTemp = MCU_GetMCU2Data()->mcuTemp,
            .dcMainWireOverVoltFault = MCU_GetMCU2Data()->dcMainWireOverVoltFault,
            .motorPhaseCurrFault = MCU_GetMCU2Data()->motorPhaseCurrFault,
            .mcuOverHotFault = MCU_GetMCU2Data()->mcuOverHotFault,
            .resolverFault = MCU_GetMCU2Data()->resolverFault,
            .phaseCurrSensorFault = MCU_GetMCU2Data()->phaseCurrSensorFault,
            .motorOverSpdFault = MCU_GetMCU2Data()->motorOverSpdFault,
            .drvMotorOverHotFault = MCU_GetMCU2Data()->drvMotorOverHotFault,
            .dcMainWireOverCurrFault = MCU_GetMCU2Data()->dcMainWireOverCurrFault,
            .drvMotorOverCoolFault = MCU_GetMCU2Data()->drvMotorOverCoolFault,
            .mcuMotorSystemState = MCU_GetMCU2Data()->mcuMotorSystemState,
            .mcuTempSensorState = MCU_GetMCU2Data()->mcuTempSensorState,
            .motorTempSensorState = MCU_GetMCU2Data()->motorTempSensorState,
            .dcVoltSensorState = MCU_GetMCU2Data()->dcVoltSensorState,
            .dcLowVoltWarning = MCU_GetMCU2Data()->dcLowVoltWarning,
            .mcu12VLowVoltWarning = MCU_GetMCU2Data()->mcu12VLowVoltWarning,
            .motorStallFault = MCU_GetMCU2Data()->motorStallFault,
            .motorOpenPhaseFault = MCU_GetMCU2Data()->motorOpenPhaseFault,
            .mcuWarningLevel = MCU_GetMCU2Data()->mcuWarningLevel,

            .mcuVoltage = MCU_GetMCU3Data()->mcuVoltage,
            .mcuCurrent = MCU_GetMCU3Data()->mcuCurrent,
            .motorPhaseCurr = MCU_GetMCU3Data()->motorPhaseCurr,
        };
        taskEXIT_CRITICAL();

        uint8_t* serializedData = (uint8_t*) &telemetryData;
        CAN_ISOTP_Send(TELEMETRY_CAN_ID, serializedData, sizeof(TelemetryData));

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(TELEMETRY_PERIOD_MS)); // Delay until the next telemetry update
    }
}

TelemetryData const* Telemetry_GetData() {
    return &telemetryData;
}
