#include "peripherals/timer.h"
#include "callbacks.h"
#include <arduino_freertos.h>

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1
#define THREAD_CAN_TELEMETRY_PRIORITY 2

void Timer_StartADCScan(){
    readPinTimer.begin(StartADCScanCallback, 1000);
    // readPinTimer.begin(updateADCDataForRaspiTesting, 900);
    // xTaskCreate(updateADCDataForRaspiTesting, "threadADCUpdate", THREAD_CAN_STACK_SIZE, NULL, THREAD_CAN_TELEMETRY_PRIORITY, NULL);
}
