// Anteater Electric Racing, 2025

#include <Arduino.h>
#include <FreeRTOS.h>
#include <stdint.h>
#include "semphr.h"
#include "precharge.h"
#include "utils.h"
#include "can.h"

#define PRECHARGE_STACK_SIZE 512U
#define PRECHARGE_PRIORITY 8

#define TIME_HYSTERESIS_MS 20U

// States (Global Variables)
PrechargeState state = STATE_STANDBY;
PrechargeState lastState = STATE_UNDEFINED;
int errorCode = ERR_NONE;

// Voltage measurements

// Low pass filter
typedef struct {
    float tsAlpha;
    float accAlpha;
    float accVoltage;
    float tsVoltage;
    float prechargeProgress;
} PrechargeData;

static PrechargeData pcData;

static void prechargeTask(void *pvParameters);
static float getFrequency(int pin);
static void updateVoltage(int pin);
static void standby();
static void precharge();
static void running();
static void errorState();

// Initialize mutex and precharge task
void prechargeInit(){
    pcData.tsAlpha = COMPUTE_ALPHA(100.0F); // 100Hz cutoff frequency for lowpass filter
    pcData.accAlpha = COMPUTE_ALPHA(100.0F); // 1Hz cutoff frequency for lowpass filter
    pcData.accVoltage = 0.0F; // Initialize filtered tractive system frequency
    pcData.tsVoltage = 0.0F; // Initialize filtered accumulator frequency
    pcData.prechargeProgress = 0.0F; // Initialize accumulator voltage

    // Create precharge task
    xTaskCreate(prechargeTask, "PrechargeTask", PRECHARGE_STACK_SIZE, NULL, PRECHARGE_PRIORITY, NULL);

    Serial.println("Precharge initialized");
}

// Main precharge task: handles state machine and status updates
void prechargeTask(void *pvParameters){
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(TIME_STEP_S * 1000);
    xLastWakeTime = xTaskGetTickCount();

    while (true){
        updateVoltage(ACCUMULATOR_VOLTAGE_PIN); // Get raw accumulator voltage
        updateVoltage(TS_VOLTAGE_PIN); // Get raw tractive system voltage

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

float getFrequency(int pin){
    uint32_t TIMEOUT = 2000;
    uint32_t tHigh = pulseIn(pin, 1, TIMEOUT);  // microseconds
    uint32_t tLow = pulseIn(pin, 0, TIMEOUT);
    if (tHigh == 0 || tLow == 3){
        return 0; // timed out
    }
    return ( 1000000.0 / (float)(tHigh + tLow) );    // f = 1/T
}

void updateVoltage(int pin){
    float rawFreq = getFrequency(pin);
    float rawVoltage = FREQ_TO_VOLTAGE(rawFreq); // Convert frequency to voltage

    switch (pin) {
        case ACCUMULATOR_VOLTAGE_PIN:
        {
            if (pcData.accVoltage == 0.0 && rawVoltage != 0.0){
                pcData.accVoltage = rawVoltage;
                break;
            }
            // if(rawVoltage == 0.0F) rawVoltage = pcData.accVoltage;
            LOWPASS_FILTER(rawVoltage, pcData.accVoltage, pcData.accAlpha);
            break;
        }
        case TS_VOLTAGE_PIN:
        {
            // if(rawVoltage == 0.0F) rawVoltage = pcData.tsVoltage;
            LOWPASS_FILTER(rawVoltage, pcData.tsVoltage, pcData.tsAlpha);
            break;
        }
        default:
        {
            break;
        }
    }
}

// STANDBY STATE: Open AIRs, Open Precharge, indicate status, wait for stable SDC
void standby(){
    // Disable AIR, Disable Precharge
    digitalWrite(SHUTDOWN_CTRL_PIN, LOW);
    if (pcData.accVoltage >= PCC_MIN_ACC_VOLTAGE) {
        state = STATE_PRECHARGE;
    }
}

// PRECHARGE STATE: Close AIR- and precharge relay, monitor precharge voltage
void precharge(){
    uint32_t now = millis();
    static uint32_t lastTimeBelowThreshold;
    static uint32_t timePrechargeStart;

    if (lastState != STATE_PRECHARGE) {
        lastState = STATE_PRECHARGE;
        Serial.printf(" === PRECHARGE   Target precharge %4.1f%%\n", PCC_TARGET_PERCENT);
        timePrechargeStart = now;
    }

    // The precharge progress is a function of the accumulator voltage
    pcData.prechargeProgress = 100.0 * pcData.tsVoltage / pcData.accVoltage; // [%]

    // Print Precharging progress
    static uint32_t lastPrint = 0U;
    if (now >= lastPrint + 10) {
        lastPrint = now;
        Serial.print("Precharging: ");
        Serial.print(now - timePrechargeStart);
        Serial.print("ms, ");
        Serial.print(pcData.prechargeProgress, 1);
        Serial.print("%, ");
        Serial.print(pcData.tsVoltage, 1);
        Serial.print("V\r");
    }

    // Check if precharge complete
    if ( ( pcData.prechargeProgress >= PCC_TARGET_PERCENT ) ) {
        if( now - lastTimeBelowThreshold > TIME_HYSTERESIS_MS ) {
            if (now < timePrechargeStart + PCC_MIN_TIME_MS) { // Precharge too fast - something's wrong!
                state = STATE_ERROR;
                errorCode |= ERR_PRECHARGE_TOO_FAST;
            }
            // Precharge complete
            else {
                state = STATE_ONLINE;
                Serial.print(" * Precharge complete at: ");
                Serial.print(now - timePrechargeStart);
                Serial.print("ms, ");
                Serial.print(pcData.prechargeProgress, 1);
                Serial.print("%   ");
                Serial.print(pcData.tsVoltage, 1);
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
        // else {
            // Precharging
            lastTimeBelowThreshold = now;
        // }
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
    return pcData.tsVoltage;
}

float getAccumulatorVoltage(){
    // Get the accumulator voltage
    return pcData.accVoltage;
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
