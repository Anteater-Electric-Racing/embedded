#include <unity.h>
#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "vehicle/motor.h"
#include "../test_desktop/test_apps.h"
#include "../test_desktop/test_bse.h"
#include "../test_desktop/test_faults.h"
#include "../test_desktop/test_motor.h"

void setUp(void) {
    // set stuff up here
    APPS_Init();
    BSE_Init();
    Faults_Init();
    Motor_Init();
}

void tearDown(void) {
    // clean stuff up here
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    run_APPS_tests();
    run_BSE_tests();
    run_Faults_tests();
    run_Motor_tests();

    UNITY_END();
    return 0;
}