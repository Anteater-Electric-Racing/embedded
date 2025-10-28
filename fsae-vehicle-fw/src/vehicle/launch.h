#pragma once

typedef enum {
    LAUNCH_STATE_OFF,
    LAUNCH_STATE_ON,
    LAUNCH_STATE_FAULT
} LaunchState;

void LaunchControl_Init();
float LaunchControl_Update(float wheelSpeedFL, float wheelSpeedFR, 
                          float wheelSpeedRL, float wheelSpeedRR);
LaunchState Launch_getState();