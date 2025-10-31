mod can;
mod send;
mod utils;
use crate::send::{send_message};
use crate::can::{TelemetryData, MotorState, MotorRotateDirection, MCUMainState, MCUWorkMode, MCUWarningLevel};
use crate::utils::verify_influx_write;

#[tokio::main]
async fn main() {
    let test_packet = TelemetryData {
        apps_travel: 1.0,
        motor_speed: 1.0,
        motor_torque: 1.0,
        max_motor_torque: 1.0,
        motor_direction: MotorRotateDirection::DirectionForward,
        motor_state: MotorState::MotorStateDriving,
        mcu_main_state: MCUMainState::StateRun,
        mcu_work_mode: MCUWorkMode::WorkModeStandby,
        mcu_voltage: 1.0,
        mcu_current: 1.0,
        motor_temp: 1,
        mcu_temp: 1,
        dc_main_wire_over_volt_fault: false,
        dc_main_wire_over_curr_fault: false,
        motor_over_spd_fault: false,
        motor_phase_curr_fault: false,
        motor_stall_fault: false,
        mcu_warning_level: MCUWarningLevel::ErrorNone,
        debug_0: 0.0,
        debug_1: 0.0,
        debug_2: 0.0,
        debug_3: 0.0,
    };

    println!("Sending TelemetryData test packet to influxdb3");
    send_message(test_packet.clone()).await;
    println!("finished sending, now verifying...");
    verify_influx_write(&test_packet).await;

}

