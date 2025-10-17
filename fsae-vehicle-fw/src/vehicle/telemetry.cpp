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


        .motorSpeed = 0.0F,
        .motorTorque = 0.0F,
        .maxMotorTorque = 0.0F,

        .motorDirection = DIRECTION_STANDBY,
        .motorState = MOTOR_STATE_OFF,

        .mcuMainState = STATE_STANDBY,
        .mcuWorkMode = WORK_MODE_STANDBY,

        .mcuVoltage = 0.0F,
        .mcuCurrent = 0.0F,
        .motorTemp = 25,
        .mcuTemp = 25,

        .dcMainWireOverVoltFault = false,
        .dcMainWireOverCurrFault = false,
        .motorOverSpdFault = false,
        .motorPhaseCurrFault = false,
        .motorStallFault = false,

        .mcuWarningLevel = ERROR_NONE,

    };
}

void threadTelemetry(void *pvParameters){
    static TickType_t lastWakeTime = xTaskGetTickCount(); // Initialize the last wake time
    while(true){
        taskENTER_CRITICAL(); // Enter critical section
        telemetryData = {
            .APPS_Travel = APPS_GetAPPSReading(),
           // .BSEFront_PSI = BSE_GetBSEReading()->bseFront_PSI,
           // .BSERear_PSI = BSE_GetBSEReading()->bseFront_PSI,
           // .accumulatorVoltage = 0.0F, // TODO: Replace with actual accumulator voltage reading
           // .accumulatorTemp_F = 0.0F, // TODO: Replace with actual accumulator temperature reading


            .motorSpeed = MCU_GetMCU1Data()->motorSpeed,
            .motorTorque = MCU_GetMCU1Data()->motorTorque,
            .maxMotorTorque = MCU_GetMCU1Data()->maxMotorTorque,
            .maxMotorBrakeTorque = MCU_GetMCU1Data()->maxMotorBrakeTorque,

            .motorDirection = MCU_GetMCU1Data()->motorDirection,
            .motorState = Motor_GetState(),
            .mcuMainState = MCU_GetMCU1Data()->mcuMainState,
            .mcuWorkMode = MCU_GetMCU1Data()->mcuWorkMode,

            .mcuVoltage = MCU_GetMCU3Data()->mcuVoltage,
            .mcuCurrent = MCU_GetMCU3Data()->mcuCurrent,
            .motorTemp = MCU_GetMCU2Data()->motorTemp,
            .mcuTemp = MCU_GetMCU2Data()->mcuTemp,

            .dcMainWireOverVoltFault = MCU_GetMCU2Data()->dcMainWireOverVoltFault,
            .dcMainWireOverCurrFault = MCU_GetMCU2Data()->dcMainWireOverCurrFault,
            .motorOverSpdFault = MCU_GetMCU2Data()->motorOverSpdFault,
            .motorPhaseCurrFault = MCU_GetMCU2Data()->motorPhaseCurrFault,

            .motorStallFault = MCU_GetMCU2Data()->motorStallFault,
            .mcuWarningLevel = MCU_GetMCU2Data()->mcuWarningLevel,

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
