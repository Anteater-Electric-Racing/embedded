// Anteater Electric Racing, 2025

#include <unity.h>
#include "vehicle/motor.h"
#include "vehicle/apps.h"
#include <iostream>

void test_Motor_Init(void) {
    TEST_ASSERT_EQUAL(MOTOR_STATE_OFF, Motor_GetState());
}

// void test_Motor_UpdateState_Precharging(void) {
//     // still implementing motor state logic
//     TEST_ASSERT_TRUE(false);
// }
// void test_Motor_UpdateState_Idle(void) {
//     // still implementing motor state logic
//     TEST_ASSERT_TRUE(false);
// }
// void test_Motor_UpdateState_Driving(void) {
//     // still implementing motor state logic
//     TEST_ASSERT_TRUE(false);
// }
// void test_Motor_UpdateState_Fault(void) {
//     // still implementing motor state logic
//     TEST_ASSERT_TRUE(false);
// }

void test_Motor_GetTorqueDemand(void) {
    APPS_UpdateData(100, 200);
    Motor_UpdateMotor(); // manually calling to simulate adc scan

    // reading set to APPS 1, not using low pass filter
    float throttle = APPS_3V3_PERCENTAGE(APPS_ADC_TO_VOLTAGE(100));

    TEST_ASSERT_EQUAL(throttle, Motor_GetTorqueDemand());
}

void test_Motor_SetFaultState(void) {
    Motor_SetFaultState();
    TEST_ASSERT_EQUAL(MOTOR_STATE_FAULT, Motor_GetState());
    TEST_ASSERT_EQUAL(0.0F, Motor_GetTorqueDemand());
}

int run_Motor_tests() {
    RUN_TEST(test_Motor_Init);
    // RUN_TEST(test_Motor_UpdateState_Precharging);
    // RUN_TEST(test_Motor_UpdateState_Idle);
    // RUN_TEST(test_Motor_UpdateState_Driving);
    // RUN_TEST(test_Motor_UpdateState_Fault);
    RUN_TEST(test_Motor_GetTorqueDemand);
    RUN_TEST(test_Motor_SetFaultState);
    return 0;
}