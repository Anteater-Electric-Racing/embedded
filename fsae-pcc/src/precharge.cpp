#include <Arduino.h>
#include <FreeRTOS.h>
#include "semphr.h"
#include "precharge.h"
#include "states.h"
#include "moving-average.h"

#define PRECHARGE_STACK_SIZE 512
#define PRECHARGE_PRIORITY 3

// Constants
const float MIN_EXPECTED = 500; // [ms] Minimum expected precharge time
const float MAX_EXPECTED = 3000; // Max expected precharge time, TODO: (can change later)
const float TARGET_PERCENT = 90.0; // TSV = 0.9 * ACV
const unsigned int SETTLING_TIME = 200; // [ms] Precharge amount must be over TARGET_PERCENT for this long before we consider precharge complete
const float MIN_SDC_VOLTAGE = 11.0; // [V] Minimum voltage for shutdown circuit

char lineBuffer[50]; // Buffer for serial output

// Mutex to protect shared resource
SemaphoreHandle_t stateMutex;

// States (Global Variables)
volatile STATEVAR state = STATE_STANDBY;
volatile STATEVAR lastState = STATE_UNDEFINED;
int errorCode = ERR_NONE;

StatusLight statusLED[4] {{ STATUS_LED[0] },
                          { STATUS_LED[1] },
                          { STATUS_LED[2] },
                          { STATUS_LED[3] }};

// Exponential Moving Averages
MovingAverage TSV_Average(0, 0.1); // 10% of new value is added to the average (more filtering)
MovingAverage ACV_Average(0, 0.1);
MovingAverage SDC_Average(0, 0.5); // 50% of new value is added to the average (less filtering -> quicker response)

// Initialize mutex and precharge task
void prechargeInit(){
    // Create mutex for PCC state
    stateMutex = xSemaphoreCreateMutex();

    // Create precharge task
    xTaskCreate(prechargeTask,          // Task function
                "PrechargeTask",        // Task name
                PRECHARGE_STACK_SIZE,   // Stack size
                NULL,                   // Parameters
                PRECHARGE_PRIORITY,     // Priority
                NULL);                  // Task handle

    Serial.println("Precharge initialized");
}

// Main precharge task: handles state machine and status updates
void prechargeTask(void *pvParameters){

    // Stores the last time the last time task was ran
    TickType_t xLastWakeTime;
    // 10ms task freq
    const TickType_t xFrequency = pdMS_TO_TICKS(10);
    // Get current time
    xLastWakeTime = xTaskGetTickCount();

    while (1){
        Serial.println("In precharge task");
        if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE){
            switch(state){
                case STATE_STANDBY:
                    Serial.println("In standby state");
                    standby();
                    break;

                case STATE_PRECHARGE:
                    Serial.println("In precharge state");
                    precharge();
                    break;

                case STATE_ONLINE:
                    Serial.println("In online state");
                    running();
                    break;

                case STATE_ERROR:
                    Serial.println("In error state");
                    errorState();
                    break;

                default: // Undefined state
                    state = STATE_ERROR;
                    errorCode |= ERR_STATE_UNDEFINED;
                    errorState();
            }
            // Update status
            updateStatusLeds();

            // Give mutex after critical section
            xSemaphoreGive(stateMutex);
        }
        // Wait for next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        delay(1000);
    }
}

// Return current precharge state
STATEVAR getPrechargeState(){
    STATEVAR currentPrechargeState;

    if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE){
        currentPrechargeState = state;
        xSemaphoreGive(stateMutex);
    }

    return currentPrechargeState;
}

// Obtain current error information
int getPrechargeError(){
    int currentPrechargeError;

    if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE){
        currentPrechargeError = errorCode;
        xSemaphoreGive(stateMutex);
    }

    return currentPrechargeError;
}

// STANDBY STATE: Open AIRs, Open Precharge, indicate status, wait for stable SDC
void standby(){
static unsigned long epoch;
    if (lastState != STATE_STANDBY) {
        lastState = STATE_STANDBY;
        statusLEDsOff();
        statusLED[0].on();
        Serial.println(" === STANDBY");
        Serial.println("* Waiting for stable shutdown circuit");
        epoch = millis(); // make sure to reset if we've circled back to standby

        // Reset moving averages
        TSV_Average.reset();
        ACV_Average.reset();
        SDC_Average.reset();
    }

    // Disable AIR, Disable Precharge
    digitalWrite(PRECHARGE_CTRL_PIN, LOW);
    digitalWrite(SHUTDOWN_CTRL_PIN, LOW);

    // Check for stable shutdown circuit
    const unsigned int WAIT_TIME = 200; // ms to wait for stable voltage
    if (SDC_Average.value() >= MIN_SDC_VOLTAGE){
        if (millis() > epoch + WAIT_TIME){
        state = STATE_PRECHARGE;
        }
    } else {
        epoch = millis(); // reset timer
    }
}

// PRECHARGE STATE: Close AIR- and precharge relay, monitor precharge voltage
void precharge(){
    unsigned long now = millis();
    static unsigned long epoch;
    static unsigned long timePrechargeStart;

    if (lastState != STATE_PRECHARGE){
        digitalWrite(PRECHARGE_CTRL_PIN, HIGH);
        lastState = STATE_PRECHARGE;
        statusLEDsOff();
        statusLED[1].on();
        sprintf(lineBuffer, " === PRECHARGE   Target precharge %4.1f%%\n", TARGET_PERCENT);
        Serial.print(lineBuffer);
        epoch = now;
        timePrechargeStart = now;
    }

    // Sample the voltages and update moving averages
    const unsigned long samplePeriod = 10; // [ms] Period to measure voltages
    static unsigned long lastSample = 0;
    if (now > lastSample + samplePeriod){  // samplePeriod and movingAverage alpha value will affect moving average response.
        lastSample = now;
        // TODO: Fix this below to use the frequency-to-voltage converter
        //ACV_Average.update(getAccuVoltage());
        //TSV_Average.update(getTsVoltage());
    }
    double acv = ACV_Average.value();
    double tsv = TSV_Average.value();

    // The precharge progress is a function of the accumulator voltage
    double prechargeProgress = 100.0 * tsv / acv; // [%]

    // Print Precharging progress
    static unsigned long lastPrint = 0;
    if (now >= lastPrint + 100) {
        lastPrint = now;
        sprintf(lineBuffer, "%5lums %4.1f%%   %5.1fV\n", now-timePrechargeStart, prechargeProgress, TSV_Average.value());
        Serial.print(lineBuffer);
    }

    // Check if precharge complete
    if ( prechargeProgress >= TARGET_PERCENT ) {
        // Precharge complete
        if (now > epoch + SETTLING_TIME){
            state = STATE_ONLINE;
            sprintf(lineBuffer, "* Precharge complete at: %2.0f%%   %5.1fV\n", prechargeProgress, TSV_Average.value());
            Serial.print(lineBuffer);
        }
        else if (now < timePrechargeStart + MIN_EXPECTED && now > epoch + SETTLING_TIME) {    // Precharge too fast - something's wrong!
            state = STATE_ERROR;
            errorCode |= ERR_PRECHARGE_TOO_FAST;
        }

    } else {
        // Precharging
        epoch = now;

        if (now > timePrechargeStart + MAX_EXPECTED) {       // Precharge too slow - something's wrong!
            state = STATE_ERROR;
            errorCode |= ERR_PRECHARGE_TOO_SLOW;
        }
    }
}

// ONLINE STATE: Close AIR+ to connect ACC to TS, Open Precharge relay, indicate status
void running(){
    const unsigned int T_OVERLAP = 500; // [ms] Time to overlap the switching of AIR and Precharge
    static unsigned long epoch;
    unsigned long now = millis();

    if (lastState != STATE_ONLINE) {
        lastState = STATE_ONLINE;
        statusLEDsOff();
        statusLED[2].on();
        Serial.println(" === ONLINE");
        Serial.println("* Precharge complete, closing AIR+");
        epoch = now;
    }

    // Close AIR+
    digitalWrite(SHUTDOWN_CTRL_PIN, HIGH);
    if (now > epoch + T_OVERLAP) digitalWrite(PRECHARGE_CTRL_PIN, LOW);

    // Check for errors
    if (errorCode != ERR_NONE){
        state = STATE_ERROR;
    }
}

// ERROR STATE: Indicate error, open AIRs and precharge relay
void errorState(){
    digitalWrite(PRECHARGE_CTRL_PIN, LOW);
    digitalWrite(SHUTDOWN_CTRL_PIN, LOW);

    if (lastState != STATE_ERROR){
        lastState = STATE_ERROR;
        statusLEDsOff();
        statusLED[3].update(50,50); // Strobe STS LED
        Serial.println(" === ERROR");

        // Display errors: Serial and Status LEDs
        if (errorCode == ERR_NONE){
        Serial.println("   *Error state, but no error code logged...");
        }
        if (errorCode & ERR_PRECHARGE_TOO_FAST) {
        Serial.println("   *Precharge too fast. Suspect wiring fault / chatter in shutdown circuit.");
        statusLED[0].on();
        }
        if (errorCode & ERR_PRECHARGE_TOO_SLOW) {
        Serial.println("   *Precharge too slow. Potential causes:\n   - Wiring fault\n   - Discharge is stuck-on\n   - Target precharge percent is too high");
        statusLED[1].on();
        }
        if (errorCode & ERR_STATE_UNDEFINED) {
        Serial.println("   *State not defined in The State Machine.");
        }
    }
}

// Loop through the array and call update.
void updateStatusLeds() {
  for (uint8_t i=0; i<(sizeof(statusLED)/sizeof(*statusLED)); i++){
    statusLED[i].update();
  }
}
void statusLEDsOff() {
  for (uint8_t i=0; i<(sizeof(statusLED)/sizeof(*statusLED)); i++){
    statusLED[i].off();
  }
}
