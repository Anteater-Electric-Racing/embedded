use std::time::Duration;

use crate::send::{send_message, Reading};
use rand::Rng;
use serde::Serialize;
use tokio::time::sleep;
use tokio_socketcan_isotp::{IsoTpSocket, StandardId};

const CAN_INTERFACE: &str = "can0";

macro_rules! define_enum {
    ($name:ident, $($variant:ident = $value:expr),*) => {
        #[derive(Serialize)]
        enum $name {
            $($variant = $value),*
        }

        impl $name {
            fn from_byte(byte: u8) -> Self {
                match byte {
                    $($value => Self::$variant),*,
                    _ => panic!("Invalid value for {}", stringify!($name)),
                }
            }
        }
    };
}

define_enum!(
    MotorState,
    MotorStateOff = 0,
    MotorStatePrecharging = 1,
    MotorStateIdle = 2,
    MotorStateDriving = 3,
    MotorStateFault = 4
);

define_enum!(
    MotorRotateDirection,
    DirectionStandby = 0,
    DirectionForward = 1,
    DirectionBackward = 2,
    DirectionError = 3
);

define_enum!(
    MCUMainState,
    StateStandby = 0,
    StatePrecharge = 1,
    StatePowerReady = 2,
    StateRun = 3,
    StatePowerOff = 4
);

define_enum!(
    MCUWorkMode,
    WorkModeStandby = 0,
    WorkModeTorque = 1,
    WorkModeSpeed = 2
);

define_enum!(
    MCUWarningLevel,
    ErrorNone = 0,
    ErrorLow = 1,
    ErrorMedium = 2,
    ErrorHigh = 3
);

#[derive(Serialize)]
struct TelemetryData {
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

fn parse_bool(byte: u8) -> bool {
    byte != 0
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
            if packet.len() != 94 {
                println!("Invalid packet length: {}", packet.len());
                continue;
            }
            send_message(TelemetryData {
                apps_travel: f32::from_le_bytes(packet[0..4].try_into().unwrap()),
                bse_front_psi: f32::from_le_bytes(packet[4..8].try_into().unwrap()),
                bse_rear_psi: f32::from_le_bytes(packet[8..12].try_into().unwrap()),
                accumulator_voltage: f32::from_le_bytes(packet[12..16].try_into().unwrap()),
                accumulator_temp_f: f32::from_le_bytes(packet[16..20].try_into().unwrap()),
                motor_state: MotorState::from_byte(packet[20]),
                motor_speed: f32::from_le_bytes(packet[21..25].try_into().unwrap()),
                motor_torque: f32::from_le_bytes(packet[25..29].try_into().unwrap()),
                max_motor_torque: f32::from_le_bytes(packet[29..33].try_into().unwrap()),
                max_motor_brake_torque: f32::from_le_bytes(packet[33..37].try_into().unwrap()),
                motor_direction: MotorRotateDirection::from_byte(packet[37]),
                mcu_main_state: MCUMainState::from_byte(packet[38]),
                mcu_work_mode: MCUWorkMode::from_byte(packet[39]),
                motor_temp: i32::from_le_bytes(packet[40..44].try_into().unwrap()),
                mcu_temp: i32::from_le_bytes(packet[44..48].try_into().unwrap()),
                dc_main_wire_over_volt_fault: parse_bool(packet[48]),
                motor_phase_curr_fault: parse_bool(packet[49]),
                mcu_over_hot_fault: parse_bool(packet[50]),
                resolver_fault: parse_bool(packet[51]),
                phase_curr_sensor_fault: parse_bool(packet[52]),
                motor_over_spd_fault: parse_bool(packet[53]),
                drv_motor_over_hot_fault: parse_bool(packet[54]),
                dc_main_wire_over_curr_fault: parse_bool(packet[55]),
                drv_motor_over_cool_fault: parse_bool(packet[56]),
                mcu_motor_system_state: parse_bool(packet[57]),
                mcu_temp_sensor_state: parse_bool(packet[58]),
                motor_temp_sensor_state: parse_bool(packet[59]),
                dc_volt_sensor_state: parse_bool(packet[60]),
                dc_low_volt_warning: parse_bool(packet[61]),
                mcu_12v_low_volt_warning: parse_bool(packet[62]),
                motor_stall_fault: parse_bool(packet[63]),
                motor_open_phase_fault: parse_bool(packet[64]),
                mcu_warning_level: MCUWarningLevel::from_byte(packet[65]),
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
    // loop {
    //     let telemetry = {
    //         let mut rng = rand::rng();

    //         TelemetryData {
    //             apps_travel: rng.random_range(0.0..100.0),
    //             bse_front_psi: rng.random_range(0.0..50.0),
    //             bse_rear_psi: rng.random_range(0.0..50.0),
    //             accumulator_voltage: rng.random_range(0.0..600.0),
    //             accumulator_temp_f: rng.random_range(32.0..212.0),
    //             motor_state: MotorState::from_byte(rng.random_range(0..5)),
    //             motor_speed: rng.random_range(0.0..10000.0),
    //             motor_torque: rng.random_range(-100.0..100.0),
    //             max_motor_torque: rng.random_range(0.0..150.0),
    //             max_motor_brake_torque: rng.random_range(0.0..150.0),
    //             motor_direction: MotorRotateDirection::from_byte(rng.random_range(0..4)),
    //             mcu_main_state: MCUMainState::from_byte(rng.random_range(0..5)),
    //             mcu_work_mode: MCUWorkMode::from_byte(rng.random_range(0..3)),
    //             motor_temp: rng.random_range(-40..150),
    //             mcu_temp: rng.random_range(-40..150),
    //             dc_main_wire_over_volt_fault: rng.random_bool(0.1),
    //             motor_phase_curr_fault: rng.random_bool(0.1),
    //             mcu_over_hot_fault: rng.random_bool(0.1),
    //             resolver_fault: rng.random_bool(0.1),
    //             phase_curr_sensor_fault: rng.random_bool(0.1),
    //             motor_over_spd_fault: rng.random_bool(0.1),
    //             drv_motor_over_hot_fault: rng.random_bool(0.1),
    //             dc_main_wire_over_curr_fault: rng.random_bool(0.1),
    //             drv_motor_over_cool_fault: rng.random_bool(0.1),
    //             mcu_motor_system_state: rng.random_bool(0.5),
    //             mcu_temp_sensor_state: rng.random_bool(0.5),
    //             motor_temp_sensor_state: rng.random_bool(0.5),
    //             dc_volt_sensor_state: rng.random_bool(0.5),
    //             dc_low_volt_warning: rng.random_bool(0.1),
    //             mcu_12v_low_volt_warning: rng.random_bool(0.1),
    //             motor_stall_fault: rng.random_bool(0.1),
    //             motor_open_phase_fault: rng.random_bool(0.1),
    //             mcu_warning_level: MCUWarningLevel::from_byte(rng.random_range(0..4)),
    //             mcu_voltage: rng.random_range(0.0..600.0),
    //             mcu_current: rng.random_range(-200.0..200.0),
    //             motor_phase_curr: rng.random_range(-200.0..200.0),
    //             debug_0: rng.random_range(-1000.0..1000.0),
    //             debug_1: rng.random_range(-1000.0..1000.0),
    //             debug_2: rng.random_range(-1000.0..1000.0),
    //             debug_3: rng.random_range(-1000.0..1000.0),
    //         }
    //     };

    //     send_message(telemetry).await;

    //     sleep(Duration::from_millis(100)).await;
    // }
}
