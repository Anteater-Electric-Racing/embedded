// Anteater Electric Racing, 2025
#define TELEMETRY_CAN_ID 0x666 // Example CAN ID for telemetry messages
#define TELEMETRY_PERIOD_MS 10 // Telemetry update period in milliseconds

#include "vehicle/telemetry.h"

#include <arduino_freertos.h>

TelemetryData telemetryData;

void Telemetry_Init() {
    telemetryData = {// Fill with reasonable dummy values
                     .APPS_Travel = 0.0F,
                     .BSEFront = 0.0F,
                     .BSERear = 0.0F,

                     .imdResistance = 0.0F,
                     .imdStatus = 0,

                     // BMS Data
                     .packVoltage = 0.0F,
                     .packCurrent = 0.0F,
                     .soc = 0.0F,
                     .dischargeLimit = 0.0F,
                     .chargeLimit = 0.0F,
                     .lowCellVolt = 0.0F,
                     .highCellVolt = 0.0F,
                     .avgCellVolt = 0.0F,

                     // MCU1 data
                     .motorSpeed = 0.0F,
                     .motorTorque = 0.0F,
                     .maxMotorTorque = 0.0F,
                     .motorDirection = DIRECTION_STANDBY,
                     .motorState = MOTOR_STATE_PRECHARGING,

                     .mcuMainState = STATE_STANDBY,
                     .mcuWorkMode = WORK_MODE_STANDBY,

                     .mcuVoltage = 0.0F,
                     .motorPhaseCurrent = 0.0F,
                     .mcuCurrent = 0.0F,

                     .motorTemp = 25,
                     .mcuTemp = 25,

                     // .dcMainWireOverVoltFault = false,
                     // .dcMainWireOverCurrFault = false,
                     // .motorOverSpdFault = false,
                     // .motorPhaseCurrFault = false,
                     // .motorStallFault = false,
                     .mcuWarningLevel = ERROR_NONE,

                     .shocktravel1 = 0.0F,
                     .shocktravel2 = 0.0F,
                     .shocktravel3 = 0.0F,
                     .shocktravel4 = 0.0F,

                     .dcMainWireOverVoltFault = false,
                     .motorPhaseCurrFault = false,
                     .mcuOverHotFault = false,
                     .resolverFault = false,
                     .phaseCurrSensorFault = false,
                     .motorOverSpdFault = false,
                     .drvMotorOverHotFault = false,
                     .dcMainWireOverCurrFa = false,
                     .drvMotorOverCoolFaul = false,
                     .dcLowVoltWarning = false,
                     .mcu12VLowVoltWarning = false,
                     .motorStallFault = false,
                     .motorOpenPhaseFault = false,

                     .faultMap = 0};
}

void threadTelemetry(void *pvParameters) {
    static TickType_t lastWakeTime =
        xTaskGetTickCount(); // Initialize the last wake time
    while (true) {
        taskENTER_CRITICAL(); // Enter critical section
        telemetryData = {
            .APPS_Travel = APPS_GetAPPSReading(),

            .BSEFront = BSE_GetBSEReading()->bseFront_Reading,
            .BSERear = BSE_GetBSEReading()->bseRear_Reading,

            .imdResistance = IMD_GetInfo()->resistance,
            .imdStatus = IMD_GetInfo()->status,

            .packVoltage = BMS_GetOrionData()->packVoltage,
            .packCurrent = BMS_GetOrionData()->packCurrent,
            .soc = BMS_GetOrionData()->soc,
            .dischargeLimit = BMS_GetOrionData()->dischargeLimit,
            .chargeLimit = BMS_GetOrionData()->chargeLimit,
            .lowCellVolt = BMS_GetOrionData()->lowCellVolt,
            .highCellVolt = BMS_GetOrionData()->highCellVolt,
            .avgCellVolt = BMS_GetOrionData()->avgCellVolt,

            .motorSpeed = MCU_GetMCU1Data()->motorSpeed,
            .motorTorque = MCU_GetMCU1Data()->motorTorque,
            .maxMotorTorque = MCU_GetMCU1Data()->maxMotorTorque,
            .motorState = Motor_GetState(),
            .mcuMainState = MCU_GetMCU1Data()->mcuMainState,
            .mcuWorkMode = MCU_GetMCU1Data()->mcuWorkMode,

            .mcuVoltage = MCU_GetMCU3Data()->mcuVoltage,
            .motorPhaseCurrent = MCU_GetMCU3Data()->motorPhaseCurr,
            .mcuCurrent = MCU_GetMCU3Data()->mcuCurrent,
            .motorTemp = MCU_GetMCU2Data()->motorTemp,
            .mcuTemp = MCU_GetMCU2Data()->mcuTemp,

            .mcuWarningLevel = MCU_GetMCU2Data()->mcuWarningLevel,

            .shocktravel1 = Linpot_GetData()->shockTravel1_mm,
            .shocktravel2 = Linpot_GetData()->shockTravel2_mm,
            .shocktravel3 = Linpot_GetData()->shockTravel3_mm,
            .shocktravel4 = Linpot_GetData()->shockTravel4_mm,

            .dcMainWireOverVoltFault =
                MCU_GetMCU2Data()->dcMainWireOverVoltFault,
            .motorPhaseCurrFault = MCU_GetMCU2Data()->motorPhaseCurrFault,
            .mcuOverHotFault = MCU_GetMCU2Data()->mcuOverHotFault,
            .resolverFault = MCU_GetMCU2Data()->resolverFault,
            .phaseCurrSensorFault = MCU_GetMCU2Data()->phaseCurrSensorFault,
            .motorOverSpdFault = MCU_GetMCU2Data()->motorOverSpdFault,
            .drvMotorOverHotFault = MCU_GetMCU2Data()->drvMotorOverHotFault,
            .dcMainWireOverCurrFa = MCU_GetMCU2Data()->dcMainWireOverCurrFault,
            .drvMotorOverCoolFaul = MCU_GetMCU2Data()->drvMotorOverCoolFault,
            .dcLowVoltWarning = MCU_GetMCU2Data()->dcLowVoltWarning,
            .mcu12VLowVoltWarning = MCU_GetMCU2Data()->mcu12VLowVoltWarning,
            .motorStallFault = MCU_GetMCU2Data()->motorStallFault,
            .motorOpenPhaseFault = MCU_GetMCU2Data()->motorOpenPhaseFault,

            .faultMap = (int32_t)Faults_GetFaults()};
        taskEXIT_CRITICAL();

        // Serial.print(telemetryData.faultMap);
        // Serial.print(" | ");
        // Serial.print(sizeof(TelemetryData));
        // Serial.print("\r");

        uint8_t *serializedData = (uint8_t *)&telemetryData;
        CAN_ISOTP_Send(TELEMETRY_CAN_ID, serializedData, sizeof(TelemetryData));

        vTaskDelayUntil(
            &lastWakeTime,
            pdMS_TO_TICKS(
                TELEMETRY_PERIOD_MS)); // Delay until the next telemetry update
    }
}

TelemetryData const *Telemetry_GetData() { return &telemetryData; }
