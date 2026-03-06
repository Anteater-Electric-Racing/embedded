// Anteater Electric Racing, 2025

#include "ifl100-36.h"

#include "arduino_freertos.h"

#include "peripherals/can.h"

#include "vehicle/telemetry.h"

#include "utils/utils.h"

#include "pcc_receive.h"

#include "queue.h"


#define THREAD_MCU_STACK_SIZE 128
#define THREAD_MCU_PRIORITY 1

static QueueHandle_t canRxQueue;

static void threadMCU(void *pvParameters);

static TickType_t xLastWakeTime;

typedef struct {
    uint32_t rx_id;
    uint64_t rx_data;
} CANMessage_t;

static MCU1Data mcu1Data;
static MCU2Data mcu2Data;
static MCU3Data mcu3Data;

static OrionBMSData bmsData;

void MCU_Init() {
    // Fill with reasonable dummy values
    mcu1Data = {.motorSpeed = 0.0F,
                .motorTorque = 0.0F,
                .maxMotorTorque = MOTOR_MAX_TORQUE, // Max torque in Nm
                .maxMotorBrakeTorque =
                    MOTOR_MAX_TORQUE, // Max brake torque in Nm
                .motorDirection = DIRECTION_STANDBY,
                .mcuMainState = STATE_STANDBY,
                .mcuWorkMode = WORK_MODE_STANDBY};

    mcu2Data = {.motorTemp = 25, // Default temperature in C
                .mcuTemp = 25,   // Default temperature in C
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
                .mcuWarningLevel = ERROR_NONE};

    mcu3Data = {
        .mcuVoltage = 0.0F,    // Default voltage in V
        .mcuCurrent = 16.0F,   // Default current in A
        .motorPhaseCurr = 3.0F // Default phase current in A
    };

    bmsData = {.packCurrent = 0.0F,
               .packVoltage = 0.0F,
               .soc = 0.0F,
               .relayState = 0,
               .dischargeLimit = 0.0F,
               .chargeLimit = 0.0F,
               .highTemp = 25,
               .lowTemp = 25};

    canRxQueue = xQueueCreate(10, sizeof(CANMessage_t));
    // Initialize the motor thread
    xTaskCreate(threadMCU, "threadMCU", THREAD_MCU_STACK_SIZE, NULL,
                THREAD_MCU_PRIORITY, NULL);
}


// ISR - Called by hardware when CAN message arrives
void CAN_RxInterruptHandler(uint32_t id, uint64_t data) {
    CANMessage_t msg = {.rx_id = id, .rx_data = data};
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Put message in queue from ISR
    xQueueSendFromISR(canRxQueue, &msg, &xHigherPriorityTaskWoken);
    // Wake up the processing task if needed
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void threadMCU(void *pvParameters) {
    CANMessage_t msg;
    while (true) {
        // CPU sleeps here until message arrives
        if (xQueueReceive(canRxQueue, &msg, portMAX_DELAY) == pdTRUE) {
            switch (msg.rx_id) {
                case mMCU1_ID: {
                    MCU1 mcu1 = {0};
                    memcpy(&mcu1, &msg.rx_data, sizeof(mcu1));

                    int8_t torqueDirection =
                        0; // 1 if power drive state, -1 if generate electricity state
                    if (mcu1.MCU_MotorState == 1)
                        torqueDirection = 1;
                    else if (mcu1.MCU_MotorState == 2)
                        torqueDirection = -1;

                    taskENTER_CRITICAL(); // Enter critical section
                    mcu1Data = {
                        .motorSpeed = CHANGE_ENDIANESS_16(mcu1.MCU_ActMotorSpd) *
                                    0.25F, // convert to RPM
                        .motorTorque = mcu1.MCU_ActMotorTq * 0.392F * torqueDirection *
                                    MOTOR_MAX_TORQUE, // convert to Nm
                        .maxMotorTorque = mcu1.MCU_MaxMotorTq * 0.392F *
                                        MOTOR_MAX_TORQUE, // convert to Nm
                        .maxMotorBrakeTorque = mcu1.MCU_MaxMotorBrakeTq * 0.392F *
                                            MOTOR_MAX_TORQUE, // convert to Nm
                        .motorDirection =
                            (MotorRotateDirection)mcu1.MCU_MotorRotateDirection,
                        .mcuMainState = (MCUMainState)mcu1.MCU_MotorMainState,
                        .mcuWorkMode = (MCUWorkMode)mcu1.MCU_MotorWorkMode};
                    taskEXIT_CRITICAL(); // Exit critical section
                    break;
                }
                case mMCU2_ID: {
                    MCU2 mcu2 = {0};
                    memcpy(&mcu2, &msg.rx_data, sizeof(mcu2));

                    taskENTER_CRITICAL(); // Enter critical section
                    mcu2Data = {
                        .motorTemp = mcu2.MCU_Motor_Temp - 40, // convert to C
                        .mcuTemp = mcu2.MCU_hardwareTemp - 40, // convert to C
                        .dcMainWireOverVoltFault =
                            mcu2.MCU_DCMainWireOverVoltFault ? true : false,
                        .motorPhaseCurrFault =
                            mcu2.MCU_MotorPhaseCurrFault ? true : false,
                        .mcuOverHotFault = mcu2.MCU_OverHotFault ? true : false,
                        .resolverFault = mcu2.MCU_MotorResolver_Fault ? true : false,
                        .phaseCurrSensorFault =
                            mcu2.MCU_PhaseCurrSensorState ? true : false,
                        .motorOverSpdFault = mcu2.MCU_MotorOverSpdFault ? true : false,
                        .drvMotorOverHotFault =
                            mcu2.Drv_MotorOverHotFault ? true : false,
                        .dcMainWireOverCurrFault =
                            mcu2.MCU_DCMainWireOverCurrFault ? true : false,
                        .drvMotorOverCoolFault =
                            mcu2.Drv_MotorOverCoolFault ? true : false,
                        .mcuMotorSystemState = mcu2.MCU_MotorSystemState ? true : false,
                        .mcuTempSensorState = mcu2.MCU_TempSensorState ? true : false,
                        .motorTempSensorState =
                            mcu2.MCU_MotorTempSensorState ? true : false,
                        .dcVoltSensorState = mcu2.MCU_DC_VoltSensorState ? true : false,
                        .dcLowVoltWarning = mcu2.MCU_DC_LowVoltWarning ? true : false,
                        .mcu12VLowVoltWarning =
                            mcu2.MCU_12V_LowVoltWarning ? true : false,
                        .motorStallFault = mcu2.MCU_MotorStallFault ? true : false,
                        .motorOpenPhaseFault =
                            mcu2.MCU_MotorOpenPhaseFault ? true : false,
                        .mcuWarningLevel = (MCUWarningLevel)mcu2.MCU_Warning_Level};
                    taskEXIT_CRITICAL(); // Exit critical section
                    break;
                }
                case mMCU3_ID: {
                    MCU3 mcu3 = {0};
                    memcpy(&mcu3, &msg.rx_data, sizeof(mcu3));

                    taskENTER_CRITICAL(); // Enter critical section

                    mcu3Data = {
                        .mcuVoltage = (((mcu3.MCU_DC_MainWireVolt & 0xFF) << 8) |
                                    (mcu3.MCU_DC_MainWireVolt >> 8)) *
                                    0.01F, // convert to V
                        .mcuCurrent = (((mcu3.MCU_DC_MainWireCurr & 0xFF) << 8) |
                                    (mcu3.MCU_DC_MainWireCurr >> 8)) *
                                    0.01F, // convert to A
                        .motorPhaseCurr = (((mcu3.MCU_MotorPhaseCurr & 0xFF) << 8) |
                                        (mcu3.MCU_MotorPhaseCurr >> 8)) *
                                        0.01F, // convert to A
                    };

                    taskEXIT_CRITICAL(); // Exit critical section
                    break;
                }

                case pcc_ID: {
                    // Serial.print("PCC OK?");
                    processPCCMessage(msg.rx_data);
                    break;
                }

                case mOBMS1_ID: {
                    OBMS1 raw = {0};
                    memcpy(&raw, &msg.rx_data, sizeof(raw));

                    taskENTER_CRITICAL();
                    bmsData.packCurrent = raw.packCurrent * 0.1F;
                    bmsData.packVoltage = raw.packVoltage * 0.1F;
                    bmsData.soc = raw.packSOC * 0.5F;
                    bmsData.relayState = raw.relayState;
                    taskEXIT_CRITICAL();
                    break;
                }

                case mOBMS2_ID: {
                    OBMS2 raw = {0};
                    memcpy(&raw, &msg.rx_data, sizeof(raw));

                    taskENTER_CRITICAL();
                    bmsData.dischargeLimit = (float)raw.packDCL; // Drive Current Limits
                    bmsData.chargeLimit = (float)raw.packCCL;    // Regen Current Limits
                    bmsData.highTemp = raw.highTemp;
                    bmsData.lowTemp = raw.lowTemp;
                    taskEXIT_CRITICAL();
                    break;
                }

                case mOBMS3_ID: {
                    OBMS3 raw = {0};
                    memcpy(&raw, &msg.rx_data, sizeof(raw));

                    taskENTER_CRITICAL();
                    bmsData.lowCellVolt = raw.lowCellVolt * 0.0001F;
                    bmsData.highCellVolt = raw.highCellVolt * 0.0001F;
                    bmsData.avgCellVolt = raw.avgCellVolt * 0.0001F;
                    bmsData.lowCellID = raw.lowCellVoltID;
                    taskEXIT_CRITICAL();
                    break;
                }
                default: {
                    break;
                }
            }
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
    }
}

MCU1Data *MCU_GetMCU1Data() { return &mcu1Data; }

MCU2Data *MCU_GetMCU2Data() { return &mcu2Data; }

MCU3Data *MCU_GetMCU3Data() { return &mcu3Data; }

OrionBMSData *BMS_GetOrionData() { return &bmsData; }

// checksum = (byte0 + byte1 + byte2 + byte3 + byte4 + byte5 + byte6) XOR 0xFF
uint8_t ComputeChecksum(uint8_t *data) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < 7; i++) {
        sum += data[i];
    }
    return sum ^ 0xFF;
}
