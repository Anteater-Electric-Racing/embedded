#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1



#include "arduino_freertos.h"
#include "pcc_receive.h"

void processPCCMessage(uint64_t rx_data){
    taskENTER_CRITICAL();
    memcpy(&pccData, &rx_data, sizeof(PCC));
    taskEXIT_CRITICAL();

    Serial.print("Precharge Progress: ");
    Serial.println(pccData.prechargeProgress);
}



