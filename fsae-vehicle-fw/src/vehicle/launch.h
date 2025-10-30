#pragma once

typedef enum {
    LAUNCH_STATE_OFF,
    LAUNCH_STATE_ON,
    LAUNCH_STATE_FAULT
} LaunchState;

void LaunchControl_Init();
float threadLaunchControl(void *pvParameters);
LaunchState Launch_getState();