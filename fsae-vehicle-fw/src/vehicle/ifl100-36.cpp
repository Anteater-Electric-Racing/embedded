#include "ifl100-36.h"

#include "arduino_freertos.h"

#include "peripherals/can.h"
#include "utils/utils.h"

#define THREAD_MCU_STACK_SIZE 128
#define THREAD_MCU_PRIORITY 1

static void threadMCU(void *pvParameters);
static uint32_t rx_id;
static uint64_t rx_data;

void MCU_Init() {
    // Initialize the motor thread
    xTaskCreate(threadMCU, "threadMCU", THREAD_MCU_STACK_SIZE, NULL, THREAD_MCU_PRIORITY, NULL);
}

static void threadMCU(void *pvParameters) {
    while (true) {
        // Read the CAN messages
        CAN_Receive(&rx_id, &rx_data);
        switch(rx_id) {
            case mMCU1_ID:
            {
                MCU1 mcu1 = {0};
                memcpy(&mcu1, &rx_data, sizeof(mcu1));
                Serial.println("MCU1 message received");

                break;
            }
            case mMCU2_ID:
            {
                MCU2 mcu2 = {0};
                memcpy(&mcu2, &rx_data, sizeof(mcu2));
                Serial.println("MCU2 message received");
                break;
            }
            case mMCU3_ID:
            {
                MCU3 mcu3 = {0};
                memcpy(&mcu3, &rx_data, sizeof(mcu3));
                Serial.println("MCU3 message received");
                break;
            }
            default:
            {
                Serial.println("Unknown message received");
                break;
            }
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

