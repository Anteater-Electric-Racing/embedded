#include "ifl100-36.h"

#include "arduino_freertos.h"

#include "peripherals/can.h"

#include "vehicle/telemetry.h"

#include "utils/utils.h"

#define THREAD_MCU_STACK_SIZE 128
#define THREAD_MCU_PRIORITY 1

static void threadMCU(void *pvParameters);
static uint32_t rx_id;
static uint64_t rx_data;
static TelemetryData* telemetryData;

void MCU_Init() {
    // Initialize the motor thread
    xTaskCreate(threadMCU, "threadMCU", THREAD_MCU_STACK_SIZE, NULL, THREAD_MCU_PRIORITY, NULL);
}

static void threadMCU(void *pvParameters) {
    while (true) {
        // Read the CAN messages
        CAN_Receive(&rx_id, &rx_data);
        telemetryData = Telemetry_GetData();
        switch(rx_id) {
            case mMCU1_ID:
            {
                MCU1 mcu1 = {0};
                memcpy(&mcu1, &rx_data, sizeof(mcu1));
                Serial.println("MCU1 message received");
                // lock mutex i think
                uint8_t torqueDirection = 0; // 1 if power drive state, -1 if generate electricity state
                if (mcu1.MCU_MotorState == 1) {
                    torqueDirection = 1;
                } else if (mcu1.MCU_MotorState == 2) {
                    torqueDirection = -1;
                }

                telemetryData->motorSpeed = mcu1.MCU_ActMotorSpd * 0.25F; // convert to RPM
                telemetryData->motorTorque = mcu1.MCU_ActMotorTq * 0.392 * torqueDirection; // convert to Nm
                telemetryData->maxMotorTorque = mcu1.MCU_MaxMotorTq * 0.392;
                telemetryData->maxMotorBrakeTorque = mcu1.MCU_MaxMotorBrakeTq * 0.392;
                telemetryData->motorDirection = mcu1.MCU_MotorRatoteDirection;
                telemetryData->mcuMainState = mcu1.MCU_MotorMainState;
                telemetryData->mcuWorkMode = mcu1.MCU_MotorWorkMode;
                break;
            }
            case mMCU2_ID:
            {
                MCU2 mcu2 = {0};
                memcpy(&mcu2, &rx_data, sizeof(mcu2));
                Serial.println("MCU2 message received");

                telemetryData->motorTemp = mcu2.MCU_Motor_Temp - 40; // convert to C
                telemetryData->mcuTemp = mcu2.MCU_hardwareTemp - 40; // convert to C
                telemetryData->dcMainWireOverVoltFault = mcu2.MCU_DC_MainWireOverVoltFault;
                telemetryData->motorPhaseCurrFault = mcu2.MCU_MotorPhaseCurrFault;
                telemetryData->mcuOverHotFault = mcu2.MCU_OverHotFault;
                telemetryData->resolverFault = mcu2.sResolver_Fault;
                telemetryData->phaseCurrSensorFault = mcu2.MCU_PhaseCurrSensorState;
                telemetryData->motorOverSpdFault = mcu2.MCU_MotorOverSpdFault;
                telemetryData->drvMotorOverHotFault = mcu2.Drv_MotorOverHotFault;
                telemetryData->dcMainWireOverVoltFault = mcu2.MCU_DC_MainWireOverVoltFault;
                telemetryData->drvMotorOverCoolFault = mcu2.Drv_MotorOverCoolFault;
                telemetryData->mcuMotorSystemState = mcu2.MCU_MotorSystemState;
                telemetryData->mcuTempSensorState = mcu2.MCU_TempSensorState;
                telemetryData->motorTempSensorState = mcu2.MCU_MotorTempSensorState;
                telemetryData->dcVoltSensorState = mcu2.MCU_DC_VoltSensorState;
                telemetryData->dcLowVoltWarning = mcu2.MCU_DC_LowVoltWarning;
                telemetryData->mcu12VLowVoltWarning = mcu2.MCU_12V_LowVoltWarning;
                telemetryData->motorStallFault = mcu2.MCU_MotorStallFault;
                telemetryData->motorOpenPhaseFault = mcu2.MCU_MotorOpenPhaseFault;
                telemetryData->mcuWarningLevel = mcu2.MCU_Warning_Level;
                break;
            }
            case mMCU3_ID:
            {
                MCU3 mcu3 = {0};
                memcpy(&mcu3, &rx_data, sizeof(mcu3));
                Serial.println("MCU3 message received");
                telemetryData->mcuVoltage = mcu3.MCU_DC_MainWireVolt * 0.01F; // convert to V
                telemetryData->mcuCurrent = mcu3.MCU_DC_MainWireCurr * 0.01F; // convert to A
                telemetryData->motorPhaseCurr = mcu3.MCU_MotorPhaseCurr * 0.01F; // convert to A
                break;
            }
            default:
            {
                Serial.println("Unknown message received");
                break;
            }
            Telemetry_UpdateData(telemetryData);
        }
        /*
                if (rx_msg.id == mMCU1_ID) {
            // MCU1 mcu1 = {0};
            // memcpy(&mcu1, rx_msg.buf, sizeof(mcu1));
            // Serial.println("MCU1 message received");
            // Serial.print("Motor speed: ");
            // Serial.println(mcu1.sMotorSpeed);
            // Serial.print("Motor torque: ");
            // Serial.println(mcu1.sMotorTorque);
            // Serial.print("Max motor torque: ");
            // Serial.println(mcu1.sMaxMotorTq);
            // Serial.print("Max motor brake torque: ");
            // Serial.println(mcu1.sMaxMotorBrakeTq);
            // Serial.print("Motor direction: ");
            // Serial.println(mcu1.sMotorDirection);
            // Serial.print("Motor main state: ");
            // Serial.println(mcu1.sMotorMainState);
            // Serial.print("MCU1 work mode: ");
            // Serial.println(mcu1.sWorkMode);
            // Serial.print("MCU1 motor mode: ");
            // Serial.println(mcu1.sMotorMode);
        } else if (rx_msg.id == mMCU2_ID) {
            // MCU2 mcu2 = {0};
            // memcpy(&mcu2, rx_msg.buf, sizeof(mcu2));
            // Serial.println("MCU2 message received -----------------------------");
            // Serial.print("motor temp: ");
            // Serial.println(mcu2.sMotorTemp);
            // Serial.print("hardware temp: ");
            // Serial.println(mcu2.sHardwareTemp);
            // Serial.print("DC over voltage fault: ");
            // Serial.println(mcu2.sDC_OverVolt_Fault);
            // Serial.print("motor overheat fault: ");
            // Serial.println(mcu2.sMotorOvrHot_Fault);
            // Serial.print("motor over speed fault: ");
            // Serial.println(mcu2.sMotorOvrSpd_Fault);
            // Serial.print("phase current sensor fault: ");
            // Serial.println(mcu2.sPhaseCurSensor_Fault);
            // Serial.print("resolver fault: ");
            // Serial.println(mcu2.sResolver_Fault);
            // Serial.print("overheat fault: ");
            // Serial.println(mcu2.sOverHot_Fault);
            // Serial.print("motor phase current fault: ");
            // Serial.println(mcu2.sMotorPhaseCur_Fault);
            // Serial.print("DC over current fault: ");
            // Serial.println(mcu2.sDC_OverCur_Fault);
            // Serial.print("V12 low voltage fault: ");
            // Serial.println(mcu2.sV12LowVolt_Fault);
            // Serial.print("DC low voltage fault: ");
            // Serial.println(mcu2.sDC_LowVolt_Fault);
            // Serial.print("DC voltage sensor fault: ");
            // Serial.println(mcu2.sDC_VoltSensor_Fault);
            // Serial.print("motor temperature sensor fault: ");
            // Serial.println(mcu2.sMotorTempSensor_Fault);
            // Serial.print("temperature sensor fault: ");
            // Serial.println(mcu2.sTempSensor_Fault);
            // Serial.print("motor system fault: ");
            // Serial.println(mcu2.sMotorSystem_Fault);
            // Serial.print("motor overcool fault: ");
            // Serial.println(mcu2.sMotorOvrCool_Fault);
            // Serial.print("motor stall fault: ");
            // Serial.println(mcu2.sMotorStall_Fault);
            // Serial.print("motor open phase fault: ");
            // Serial.println(mcu2.sMotorOpenPhase_Fault);
            // Serial.print("warning level MCU: ");
            // Serial.println(mcu2.sWarningLevelMCU);
        } else if (rx_msg.id == mMCU3_ID) {
            // MCU3 mcu3 = {0};
            // memcpy(&mcu3, rx_msg.buf, sizeof(mcu3));
            // Serial.println("MCU3 message received -----------------------------");
            // Serial.print("DC voltage: ");
            // Serial.println(mcu3.sDC_Voltage);
            // Serial.print("DC current: ");
            // Serial.println(mcu3.sDC_Current);
            // Serial.print("motor phase current: ");
            // Serial.println(mcu3.sMotorPhaseCurrent);
*/
    }
}

// checksum = (byte0 + byte1 + byte2 + byte3 + byte4 + byte5 + byte6) XOR 0xFF
uint8_t ComputeChecksum(uint8_t* data, uint8_t length) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < length - 1; i++) {
        sum += data[i];
    }
    return sum ^ 0xFF;
}

