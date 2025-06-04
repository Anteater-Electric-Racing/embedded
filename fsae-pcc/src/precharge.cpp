// Anteater Electric Racing, 2025

#include <Arduino.h>
#include <FreeRTOS.h>
#include <stdint.h>
#include "semphr.h"
#include "precharge.h"
#include "utils.h"

#define PRECHARGE_STACK_SIZE 512
#define PRECHARGE_PRIORITY 1

// Buffer for serial output
char lineBuffer[50]; 

// States (Global Variables)
PrechargeState state = STATE_STANDBY;
PrechargeState lastState = STATE_UNDEFINED;
int errorCode = ERR_NONE;

// Low pass filter
typedef struct {
    float alpha;
    float filtered_TSF;   // filtered Transmission Side Frequency
    float filtered_ACF;   // filtered Accumulator Frequency
} LowPassFilter;

static LowPassFilter lpfValues = {0.0F, 0.0F, 0.0F};

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
    float voltage = 0.0F;

    switch (pin) {
        case ACCUMULATOR_VOLTAGE_PIN:
            LOWPASS_FILTER(rawFreq, lpfValues.filtered_ACF, lpfValues.alpha);
            voltage = FREQ_TO_VOLTAGE(lpfValues.filtered_ACF); // Convert frequency to voltage
            break;
        case TS_VOLTAGE_PIN:
            LOWPASS_FILTER(rawFreq, lpfValues.filtered_TSF, lpfValues.alpha);
            voltage = FREQ_TO_VOLTAGE(lpfValues.filtered_TSF); // Convert frequency to voltage
            break;
        default:
            Serial.println("Error: Invalid pin for voltage measurement.");
            return 0.0F; // Handle error
    }

    return voltage;
}

// Initialize mutex and precharge task
void prechargeInit(){

    lpfValues.alpha = COMPUTE_ALPHA(10.0F); // 10Hz cutoff frequency for lowpass filter

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
        // taskENTER_CRITICAL(); // Ensure atomic access to state
        switch(state){
            case STATE_STANDBY:
                // Serial.println("In standby state");
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
        // taskEXIT_CRITICAL(); // Exit critical section

        // Wait for next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// Return current precharge state
PrechargeState getPrechargeState(){
    PrechargeState currentPrechargeState;

    taskENTER_CRITICAL(); // Ensure atomic access to state
    currentPrechargeState = state;
    taskEXIT_CRITICAL(); // Exit critical section
    
    return currentPrechargeState;
}

// Obtain current error information
int getPrechargeError(){
    int currentPrechargeError;

    taskENTER_CRITICAL(); // Ensure atomic access to error code
    currentPrechargeError = errorCode;
    taskEXIT_CRITICAL(); // Exit critical section

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

        // Reset filtered values
        lpfValues.filtered_TSF = 0.0F;
        lpfValues.filtered_ACF = 0.0F;
    }

    // Disable AIR, Disable Precharge
    digitalWrite(SHUTDOWN_CTRL_PIN, LOW);

    float accVoltage = getVoltage(ACCUMULATOR_VOLTAGE_PIN); // Get raw accumulator voltage

    // Check for stable shutdown circuit
    if (accVoltage >= PCC_MIN_ACC_VOLTAGE) {
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
    float accVoltage = 0.0F;
    float tsVoltage = 0.0F;

    if (lastState != STATE_PRECHARGE) {
        lastState = STATE_PRECHARGE;
        Serial.printf(" === PRECHARGE   Target precharge %4.1f%%\n", PCC_TARGET_PERCENT);
        epoch = now;
        timePrechargeStart = now;
    }

    // Sample the voltages and update moving averages
    const uint32_t samplePeriod = 10U; // [ms] Period to measure voltages
    static uint32_t lastSample = 0U;
    if (now > lastSample + samplePeriod){  // samplePeriod and movingAverage alpha value will affect moving average response.
        lastSample = now;

        accVoltage = getVoltage(ACCUMULATOR_VOLTAGE_PIN); // Get raw accumulator voltage

        tsVoltage = getVoltage(TS_VOLTAGE_PIN); // Get raw transmission side voltage
    }

    // The precharge progress is a function of the accumulator voltage
    double prechargeProgress = 100.0 * tsVoltage / accVoltage; // [%]

    // Print Precharging progress
    static uint32_t lastPrint = 0U;
    if (now >= lastPrint + 100) {
        lastPrint = now;
        sprintf(lineBuffer, "%5lums %4.1f%%   %5.1fV\n", now-timePrechargeStart, prechargeProgress, tsVoltage);
        Serial.print(lineBuffer);
    }

    // Check if precharge complete
    if ( prechargeProgress >= PCC_TARGET_PERCENT ) {
        // Precharge complete
        if (now > epoch + PCC_SETTLING_TIME) {
            state = STATE_ONLINE;
            sprintf(lineBuffer, "* Precharge complete at: %2.0f%%   %5.1fV\n", prechargeProgress, tsVoltage);
            Serial.print(lineBuffer);
        }
        else if (now < timePrechargeStart + PCC_MIN_TIME_MS) {    // Precharge too fast - something's wrong!
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
    if (lastState != STATE_ONLINE) {
        lastState = STATE_ONLINE;
        Serial.println(" === ONLINE");
        Serial.println("* Precharge complete, closing AIR+");
    }

    // Close AIR+
    digitalWrite(SHUTDOWN_CTRL_PIN, HIGH);
}

// ERROR STATE: Indicate error, open AIRs and precharge relay
void errorState(){
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
