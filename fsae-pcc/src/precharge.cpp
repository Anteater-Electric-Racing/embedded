// Anteater Electric Racing, 2025

#include <Arduino.h>
#include <FreeRTOS.h>
#include <stdint.h>
#include "semphr.h"
#include "precharge.h"
#include "utils.h"
#include "can.h"

#define PRECHARGE_STACK_SIZE 512
#define PRECHARGE_PRIORITY 1

// States (Global Variables)
PrechargeState state = STATE_STANDBY;
PrechargeState lastState = STATE_UNDEFINED;
int errorCode = ERR_NONE;

// Voltage measurements
static float accVoltage = 0.0F;
static float tsVoltage = 0.0F;
static double prechargeProgress = 0.0F;

// Low pass filter
typedef struct {
    float tsAlpha;
    float accumAlpha;
    float filtered_TSF;   // filtered tractive system Frequency
    float filtered_ACF;   // filtered Accumulator Frequency
} LowPassFilter;

static LowPassFilter lpfValues = {0.0F, 0.0F, 0.0F};

static void prechargeTask(void *pvParameters);

float getFrequency(int pin){
    const unsigned int TIMEOUT = 700;
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
            if (lpfValues.filtered_ACF == 0.0 && rawFreq != 0.0){
                lpfValues.filtered_ACF = FREQ_TO_VOLTAGE(rawFreq);
                break;
            }
            if(rawFreq == 0.0F) rawFreq = lpfValues.filtered_ACF;
            LOWPASS_FILTER(rawFreq, lpfValues.filtered_ACF, lpfValues.accumAlpha);
            voltage = FREQ_TO_VOLTAGE(lpfValues.filtered_ACF); // Convert frequency to voltage
            break;
        case TS_VOLTAGE_PIN:
            if(rawFreq == 0.0F) rawFreq = lpfValues.filtered_TSF;
            LOWPASS_FILTER(rawFreq, lpfValues.filtered_TSF, lpfValues.tsAlpha);
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

    lpfValues.tsAlpha = COMPUTE_ALPHA(100.0F); // 100Hz cutoff frequency for lowpass filter
    lpfValues.accumAlpha = COMPUTE_ALPHA(1.0F); // 1Hz cutoff frequency for lowpass filter

    // Create precharge task
    xTaskCreate(prechargeTask, "PrechargeTask", PRECHARGE_STACK_SIZE, NULL, PRECHARGE_PRIORITY, NULL);

    Serial.println("Precharge initialized");
}

// Main precharge task: handles state machine and status updates
void prechargeTask(void *pvParameters){

    // Stores the last time the last time task was ran
    TickType_t xLastWakeTime;
    // 10ms task freq
    const TickType_t xFrequency = pdMS_TO_TICKS(1);
    // Get current time
    xLastWakeTime = xTaskGetTickCount();


    while (true){
        accVoltage = getVoltage(ACCUMULATOR_VOLTAGE_PIN); // Get raw accumulator voltage
        tsVoltage = getVoltage(TS_VOLTAGE_PIN); // Get raw tractive system voltage

        // taskENTER_CRITICAL(); // Ensure atomic access to state
        switch(state){
            case STATE_STANDBY:
                standby();
                break;

            case STATE_PRECHARGE:
                precharge();
                break;

            case STATE_ONLINE:
                running();
                break;

            case STATE_ERROR:
                errorState();
                break;

            default: // Undefined state
                state = STATE_ERROR;
                errorCode |= ERR_STATE_UNDEFINED;
                errorState();

        }
        // taskEXIT_CRITICAL(); // Exit critical section

        // Send CAN message of current PCC state
        // CAN_SendPCCMessage(millis(), state, errorCode, accVoltage, tsVoltage, prechargeProgress);

        // Wait for next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
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

    // accVoltage = getVoltage(ACCUMULATOR_VOLTAGE_PIN); // Get raw accumulator voltage
    // tsVoltage = getVoltage(TS_VOLTAGE_PIN); // Get raw tractive system voltage
    // Serial.print("Accumulator Voltage: ");
    // Serial.print(acv);
    // Serial.println(" V");
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
    static unsigned long lastTimeBelowThreshold;
    int hyst = 20;
    static unsigned long timePrechargeStart;

    if (lastState != STATE_PRECHARGE) {
        lastState = STATE_PRECHARGE;
        Serial.printf(" === PRECHARGE   Target precharge %4.1f%%\n", PCC_TARGET_PERCENT);
        epoch = now;
        timePrechargeStart = now;
    }

    // The precharge progress is a function of the accumulator voltage
    prechargeProgress = 100.0 * tsVoltage / accVoltage; // [%]

    // Print Precharging progress
    static uint32_t lastPrint = 0U;
    if (now >= lastPrint + 10) {
        lastPrint = now;
        Serial.print("Precharging: ");
        Serial.print(now - timePrechargeStart);
        Serial.print("ms, ");
        Serial.print(prechargeProgress, 1);
        Serial.print("%, ");
        Serial.print(tsVoltage, 1);
        Serial.print("V\n");
    }

    // Check if precharge complete
    if ( prechargeProgress >= PCC_TARGET_PERCENT ) {
        if ((now - lastTimeBelowThreshold) > hyst){
            if (now < timePrechargeStart + PCC_MIN_TIME_MS) {    // Precharge too fast - something's wrong!
                state = STATE_ERROR;
                errorCode |= ERR_PRECHARGE_TOO_FAST;
            }
            // Precharge complete
            else {
                state = STATE_ONLINE;
                Serial.print(" * Precharge complete at: ");
                Serial.print(now - timePrechargeStart);
                Serial.print("ms, ");
                Serial.print(prechargeProgress, 1);
                Serial.print("%   ");
                Serial.print(tsVoltage, 1);
                Serial.print("V\n");
        }
   

        }

    } else {
        if (now > timePrechargeStart + PCC_MAX_TIME_MS) {       // Precharge too slow - something's wrong!
            Serial.print(" * Precharge time: ");
            Serial.print(now - timePrechargeStart);
            Serial.print("\n");
            state = STATE_ERROR;
            errorCode |= ERR_PRECHARGE_TOO_SLOW;
        }
        // Precharging
        lastTimeBelowThreshold = now;
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

float getTSVoltage(){
    // Get the tractive system voltage
    return tsVoltage;
}

float getAccumulatorVoltage(){
    // Get the accumulator voltage
    return accVoltage;
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
