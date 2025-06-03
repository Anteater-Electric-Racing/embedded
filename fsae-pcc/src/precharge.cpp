// Anteater Electric Racing, 2025

#include <Arduino.h>
#include <FreeRTOS.h>
#include <stdint.h>
#include "semphr.h"
#include "precharge.h"
#include "utils.h"

#define PRECHARGE_STACK_SIZE 512
#define PRECHARGE_PRIORITY 3

// Buffer for serial output
char lineBuffer[50]; 

// Mutex to protect shared resource
SemaphoreHandle_t stateMutex;

// States (Global Variables)
PrechargeState state = STATE_STANDBY;
PrechargeState lastState = STATE_UNDEFINED;
int errorCode = ERR_NONE;

float getFrequency(int pin){
    const unsigned int TIMEOUT = 10000;
    unsigned int tHigh = pulseIn(pin, 1, TIMEOUT);  // microseconds
    unsigned int tLow = pulseIn(pin, 0, TIMEOUT);
    if (tHigh == 0 || tLow == 0){
        return 0; // timed out
    }
    return ( 1000000.0 / (float)(tHigh + tLow) );    // f = 1/T
}

float getVoltage(int pin){
    float rawFreq = getFrequency(pin);

    LOWPASS_FILTER(rawFreq, filteredFreq, alpha);
    
    // still need to convert freq to voltage here
    float voltage = (rawFreq / 1000.0F); // Assuming a linear conversion, adjust as needed

    return voltage;
}

// Low pass filter
struct LowPassFilter {
    static float alpha;
    static float filteredFreq;
    static float filtered_TSV;   // filtered Transmission Side Voltage
    static float filtered_ACV;   // filtered Accumulator Voltage
    static float filtered_SDC;   // filtered Shutdown Circuit Voltage
};


// Initialize mutex and precharge task
void prechargeInit(){
    // Create mutex for PCC state
    stateMutex = xSemaphoreCreateMutex();

    alpha = COMPUTE_ALPHA(1.0F); // 10Hz cutoff frequency for lowpass filter

    // Create precharge task
    xTaskCreate(prechargeTask, "PrechargeTask", PRECHARGE_STACK_SIZE, NULL, PRECHARGE_PRIORITY, NULL);

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

    while (true){
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
    }
}

// Return current precharge state
PrechargeState getPrechargeState(){
    PrechargeState currentPrechargeState;

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
        Serial.println(" === STANDBY ");
        Serial.println("* Waiting for stable shutdown circuit");
        epoch = millis(); // make sure to reset if we've circled back to standby

        // Reset moving averages
        filtered_TSV = 0.0F;
        filtered_ACV = 0.0F;
        filtered_SDC = 0.0F;
    }

    // Disable AIR, Disable Precharge
    digitalWrite(PRECHARGE_CTRL_PIN, LOW);
    digitalWrite(SHUTDOWN_CTRL_PIN, LOW);

    // Sample the raw SDC voltage each cycle
    float rawSDC = getSDCvoltage(); // TODO: replace with function that returns raw SDC value
    LOWPASS_FILTER(rawSDC, filtered_SDC, alpha);

    // Check for stable shutdown circuit
    if (filtered_SDC >= PCC_MIN_SDC_VOLTAGE){
        if (millis() > epoch + PCC_WAIT_TIME){
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
        sprintf(lineBuffer, " === PRECHARGE   Target precharge %4.1f%%\n", PCC_TARGET_PERCENT);
        Serial.print(lineBuffer);
        epoch = now;
        timePrechargeStart = now;
    }

    // Sample the voltages and update moving averages
    const uint32_t samplePeriod = 10U; // [ms] Period to measure voltages
    static uint32_t lastSample = 0U;
    if (now > lastSample + samplePeriod){  // samplePeriod and movingAverage alpha value will affect moving average response.
        lastSample = now;
        // TODO: Fix this below to use the voltage-freq converter
        float rawACV = getVoltage(ACCUMULATOR_VOLTAGE_PIN); // Get raw accumulator voltage
        LOWPASS_FILTER(rawACV, filtered_ACV, alpha);
    }
    double acv = filtered_ACV;
    double tsv = filtered_TSV;

    // The precharge progress is a function of the accumulator voltage
    double prechargeProgress = 100.0 * tsv / acv; // [%]

    // Print Precharging progress
    static uint32_t lastPrint = 0U;
    if (now >= lastPrint + 100) {
        lastPrint = now;
        sprintf(lineBuffer, "%5lums %4.1f%%   %5.1fV\n", now-timePrechargeStart, prechargeProgress, filtered_TSV);
        Serial.print(lineBuffer);
    }

    // Check if precharge complete
    if ( prechargeProgress >= PCC_TARGET_PERCENT ) {
        // Precharge complete
        if (now > epoch + PCC_SETTLING_TIME){
            state = STATE_ONLINE;
            sprintf(lineBuffer, "* Precharge complete at: %2.0f%%   %5.1fV\n", prechargeProgress, filtered_TSV);
            Serial.print(lineBuffer);
        }
        else if (now < timePrechargeStart + PCC_MIN_TIME_MS && now > epoch + PCC_MAX_TIME_MS) {    // Precharge too fast - something's wrong!
            state = STATE_ERROR;
            errorCode |= ERR_PRECHARGE_TOO_FAST;
        }

    } else {
        // Precharging
        epoch = now;

        if (now > timePrechargeStart + PCC_MAX_TIME_MS) {       // Precharge too slow - something's wrong!
            state = STATE_ERROR;
            errorCode |= ERR_PRECHARGE_TOO_SLOW;
        }
    }
}

// ONLINE STATE: Close AIR+ to connect ACC to TS, Open Precharge relay, indicate status
void running(){
    const uint32_t T_OVERLAP = 500U; // [ms] Time to overlap the switching of AIR and Precharge
    static uint32_t epoch;
    uint32_t now = millis();

    if (lastState != STATE_ONLINE) {
        lastState = STATE_ONLINE;
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
        Serial.println(" === ERROR");

        // Display errors: Serial and Status LEDs
        if (errorCode == ERR_NONE){
        Serial.println("   *Error state, but no error code logged...");
        }
        if (errorCode & ERR_PRECHARGE_TOO_FAST) {
        Serial.println("   *Precharge too fast. Suspect wiring fault / chatter in shutdown circuit.");
        }
        if (errorCode & ERR_PRECHARGE_TOO_SLOW) {
        Serial.println("   *Precharge too slow. Potential causes:\n   - Wiring fault\n   - Discharge is stuck-on\n   - Target precharge percent is too high");
        }
        if (errorCode & ERR_STATE_UNDEFINED) {
        Serial.println("   *State not defined in The State Machine.");
        }
    }
}
