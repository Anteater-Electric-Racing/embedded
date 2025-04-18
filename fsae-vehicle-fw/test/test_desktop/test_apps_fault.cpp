#include <unity.h>
#include "../lib/APPS/apps.h"
#include "../lib/APPS/faults.h"
#include "iostream"

void setUp(void) {
    // set stuff up here
    APPS_Init();
    Faults_Init();
}

void tearDown(void) {
    // clean stuff up here
}

void test_APPS_Init(void) {
    APPSData appsData = APPS_GetAPPSReading_Separate();

    TEST_ASSERT_EQUAL(0, appsData.appsReading1_Percentage);
    TEST_ASSERT_EQUAL(0, appsData.appsReading2_Percentage);
}

void test_APPS_UpdateData(void) {
    APPS_UpdateData(100, 200);

    APPSData appsData = APPS_GetAPPSReading_Separate();
    float appsReading1 = appsData.appsReading1_Percentage;
    float appsReading2 = appsData.appsReading2_Percentage;

    TEST_ASSERT_EQUAL(1, appsReading1);
    TEST_ASSERT_EQUAL(1, appsReading2);
}

void test_APPS_Fault_0(void) {
    APPS_UpdateData(0, 0); // 0% difference

    APPSData appsData = APPS_GetAPPSReading_Separate();
    float appsReading1 = appsData.appsReading1_Percentage;
    float appsReading2 = appsData.appsReading2_Percentage;

    TEST_ASSERT_FALSE(*Faults_GetFaults() & FAULT_APPS_MASK);
}

void test_APPS_Fault_09(void) {
    APPS_UpdateData(0, 368); // 9% difference

    APPSData appsData = APPS_GetAPPSReading_Separate();
    float appsReading1 = appsData.appsReading1_Percentage;
    float appsReading2 = appsData.appsReading2_Percentage;

    TEST_ASSERT_FALSE(*Faults_GetFaults() & FAULT_APPS_MASK);
}

void test_APPS_Fault_10(void) {
    APPS_UpdateData(0, 409); // 10% difference
    APPSData appsData = APPS_GetAPPSReading_Separate();
    float appsReading1 = appsData.appsReading1_Percentage;
    float appsReading2 = appsData.appsReading2_Percentage;

    std::cout << "Apps Reading 1: " << appsReading1 << std::endl;
    std::cout << "Apps Reading 2: " << appsReading2 << std::endl;
    std::cout << "Faults: " << *Faults_GetFaults() << std::endl;

    TEST_ASSERT_TRUE(*Faults_GetFaults() & FAULT_APPS_MASK);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_APPS_Init);
    RUN_TEST(test_APPS_UpdateData);
    RUN_TEST(test_APPS_Fault_0);
    RUN_TEST(test_APPS_Fault_09);
    RUN_TEST(test_APPS_Fault_10);
    UNITY_END();
    return 0;
}