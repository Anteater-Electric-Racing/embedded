use std::time::Duration;

use crate::send::{send_message, Reading};
use chrono::{DateTime, Utc};
use influxdb::InfluxDbWriteable;
use serde::Serialize;
use tokio::time::sleep;
use tokio_socketcan_isotp::{IsoTpSocket, StandardId};

const CAN_INTERFACE: &str = "can0";

#[derive(Serialize)]
enum MotorState {
    MotorStateOff,
    MotorStatePrecharging,
    MotorStateIdle,
    MotorStateDriving,
    MotorStateFault,
}

impl Into<influxdb::Type> for MotorState {
    fn into(self) -> influxdb::Type {
        influxdb::Type::Text(match self {
            MotorState::MotorStateOff => "Off".to_string(),
            MotorState::MotorStatePrecharging => "Precharging".to_string(),
            MotorState::MotorStateIdle => "Idle".to_string(),
            MotorState::MotorStateDriving => "Driving".to_string(),
            MotorState::MotorStateFault => "Fault".to_string(),
        })
    }
}

#[derive(Serialize)]
enum MotorRotateDirection {
    DirectionStandby = 0,
    DirectionForward = 1,
    DirectionBackward = 2,
    DirectionError = 3,
}

impl Into<influxdb::Type> for MotorRotateDirection {
    fn into(self) -> influxdb::Type {
        influxdb::Type::Text(match self {
            MotorRotateDirection::DirectionStandby => "Standby".to_string(),
            MotorRotateDirection::DirectionForward => "Forward".to_string(),
            MotorRotateDirection::DirectionBackward => "Backward".to_string(),
            MotorRotateDirection::DirectionError => "Error".to_string(),
        })
    }
}

#[derive(Serialize)]
enum MCUMainState {
    StateStandby = 0,
    StatePrecharge = 1,
    StatePowerReady = 2,
    StateRun = 3,
    StatePowerOff = 4,
}

impl Into<influxdb::Type> for MCUMainState {
    fn into(self) -> influxdb::Type {
        influxdb::Type::Text(match self {
            MCUMainState::StateStandby => "Standby".to_string(),
            MCUMainState::StatePrecharge => "Precharge".to_string(),
            MCUMainState::StatePowerReady => "PowerReady".to_string(),
            MCUMainState::StateRun => "Run".to_string(),
            MCUMainState::StatePowerOff => "PowerOff".to_string(),
        })
    }
}

#[derive(Serialize)]
enum MCUWorkMode {
    WorkModeStandby = 0,
    WorkModeTorque = 1,
    WorkModeSpeed = 2,
}

impl Into<influxdb::Type> for MCUWorkMode {
    fn into(self) -> influxdb::Type {
        influxdb::Type::Text(match self {
            MCUWorkMode::WorkModeStandby => "Standby".to_string(),
            MCUWorkMode::WorkModeTorque => "Torque".to_string(),
            MCUWorkMode::WorkModeSpeed => "Speed".to_string(),
        })
    }
}

#[derive(Serialize)]
enum MCUWarningLevel {
    ErrorNone = 0,
    ErrorLow = 1,
    ErrorMedium = 2,
    ErrorHigh = 3,
}

impl Into<influxdb::Type> for MCUWarningLevel {
    fn into(self) -> influxdb::Type {
        influxdb::Type::Text(match self {
            MCUWarningLevel::ErrorNone => "None".to_string(),
            MCUWarningLevel::ErrorLow => "Low".to_string(),
            MCUWarningLevel::ErrorMedium => "Medium".to_string(),
            MCUWarningLevel::ErrorHigh => "High".to_string(),
        })
    }
}

#[derive(Serialize, InfluxDbWriteable)]
struct TelemetryData {
    time: DateTime<Utc>,
    apps_travel: f32,
    bse_front_psi: f32,
    bse_rear_psi: f32,
    accumulator_voltage: f32,
    accumulator_temp_f: f32,
    motor_state: MotorState,
    motor_speed: f32,
    motor_torque: f32,
    max_motor_torque: f32,
    max_motor_brake_torque: f32,
    motor_direction: MotorRotateDirection,
    mcu_main_state: MCUMainState,
    mcu_work_mode: MCUWorkMode,
    motor_temp: i32,
    mcu_temp: i32,
    dc_main_wire_over_volt_fault: bool,
    motor_phase_curr_fault: bool,
    mcu_over_hot_fault: bool,
    resolver_fault: bool,
    phase_curr_sensor_fault: bool,
    motor_over_spd_fault: bool,
    drv_motor_over_hot_fault: bool,
    dc_main_wire_over_curr_fault: bool,
    drv_motor_over_cool_fault: bool,
    mcu_motor_system_state: bool,
    mcu_temp_sensor_state: bool,
    motor_temp_sensor_state: bool,
    dc_volt_sensor_state: bool,
    dc_low_volt_warning: bool,
    mcu_12v_low_volt_warning: bool,
    motor_stall_fault: bool,
    motor_open_phase_fault: bool,
    mcu_warning_level: MCUWarningLevel,
    mcu_voltage: f32,
    mcu_current: f32,
    motor_phase_curr: f32,
    debug_0: f32,
    debug_1: f32,
    debug_2: f32,
    debug_3: f32,
}

impl Reading for TelemetryData {
    fn topic() -> &'static str {
        "telemetry"
    }
}

pub async fn read_can() {
    loop {
        let Ok(socket) = IsoTpSocket::open(
            CAN_INTERFACE,
            StandardId::new(0x666).expect("Invalid src id"),
            StandardId::new(0x777).expect("Invalid src id"),
        ) else {
            println!("Failed to open socket");
            sleep(Duration::from_secs(1)).await;
            continue;
        };

        while let Ok(packet) = socket.read_packet().await {
            if packet.len() != 108 {
                println!("Invalid packet length: {}", packet.len());
                continue;
            }
            println!("[{}] Received packet: ", Utc::now());
            for byte in &packet {
                print!("{:02x} ", byte);
            }
            println!();
            send_message(TelemetryData {
                time: Utc::now(),
                apps_travel: f32::from_le_bytes(packet[0..4].try_into().unwrap()),
                bse_front_psi: f32::from_le_bytes(packet[4..8].try_into().unwrap()),
                bse_rear_psi: f32::from_le_bytes(packet[8..12].try_into().unwrap()),
                accumulator_voltage: f32::from_le_bytes(packet[12..16].try_into().unwrap()),
                accumulator_temp_f: f32::from_le_bytes(packet[16..20].try_into().unwrap()),
                motor_state: match packet[20] {
                    0 => MotorState::MotorStateOff,
                    1 => MotorState::MotorStatePrecharging,
                    2 => MotorState::MotorStateIdle,
                    3 => MotorState::MotorStateDriving,
                    _ => MotorState::MotorStateFault,
                },
                motor_speed: f32::from_le_bytes(packet[21..25].try_into().unwrap()),
                motor_torque: f32::from_le_bytes(packet[25..29].try_into().unwrap()),
                max_motor_torque: f32::from_le_bytes(packet[29..33].try_into().unwrap()),
                max_motor_brake_torque: f32::from_le_bytes(packet[33..37].try_into().unwrap()),
                motor_direction: match packet[37] {
                    0 => MotorRotateDirection::DirectionStandby,
                    1 => MotorRotateDirection::DirectionForward,
                    2 => MotorRotateDirection::DirectionBackward,
                    _ => MotorRotateDirection::DirectionError,
                },
                mcu_main_state: match packet[38] {
                    0 => MCUMainState::StateStandby,
                    1 => MCUMainState::StatePrecharge,
                    2 => MCUMainState::StatePowerReady,
                    3 => MCUMainState::StateRun,
                    _ => MCUMainState::StatePowerOff,
                },
                mcu_work_mode: match packet[39] {
                    0 => MCUWorkMode::WorkModeStandby,
                    1 => MCUWorkMode::WorkModeTorque,
                    _ => MCUWorkMode::WorkModeSpeed,
                },
                motor_temp: i32::from_le_bytes(packet[40..44].try_into().unwrap()),
                mcu_temp: i32::from_le_bytes(packet[44..48].try_into().unwrap()),
                dc_main_wire_over_volt_fault: packet[48] != 0,
                motor_phase_curr_fault: packet[49] != 0,
                mcu_over_hot_fault: packet[50] != 0,
                resolver_fault: packet[51] != 0,
                phase_curr_sensor_fault: packet[52] != 0,
                motor_over_spd_fault: packet[53] != 0,
                drv_motor_over_hot_fault: packet[54] != 0,
                dc_main_wire_over_curr_fault: packet[55] != 0,
                drv_motor_over_cool_fault: packet[56] != 0,
                mcu_motor_system_state: packet[57] != 0,
                mcu_temp_sensor_state: packet[58] != 0,
                motor_temp_sensor_state: packet[59] != 0,
                dc_volt_sensor_state: packet[60] != 0,
                dc_low_volt_warning: packet[61] != 0,
                mcu_12v_low_volt_warning: packet[62] != 0,
                motor_stall_fault: packet[63] != 0,
                motor_open_phase_fault: packet[64] != 0,
                mcu_warning_level: match packet[65] {
                    0 => MCUWarningLevel::ErrorNone,
                    1 => MCUWarningLevel::ErrorLow,
                    2 => MCUWarningLevel::ErrorMedium,
                    _ => MCUWarningLevel::ErrorHigh,
                },
                mcu_voltage: f32::from_le_bytes(packet[66..70].try_into().unwrap()),
                mcu_current: f32::from_le_bytes(packet[70..74].try_into().unwrap()),
                motor_phase_curr: f32::from_le_bytes(packet[74..78].try_into().unwrap()),
                debug_0: f32::from_le_bytes(packet[78..82].try_into().unwrap()),
                debug_1: f32::from_le_bytes(packet[82..86].try_into().unwrap()),
                debug_2: f32::from_le_bytes(packet[86..90].try_into().unwrap()),
                debug_3: f32::from_le_bytes(packet[90..94].try_into().unwrap()),
            })
            .await;
        }
    }
}
