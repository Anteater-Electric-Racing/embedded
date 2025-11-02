use std::time::Duration;

use crate::send::{send_message, Reading};
// use rand::Rng;
use serde::Serialize;
use tokio::time::sleep;
use tokio_socketcan_isotp::{IsoTpSocket, StandardId};

const CAN_INTERFACE: &str = "can0";

macro_rules! define_enum {
    ($name:ident, $($variant:ident = $value:expr),*) => {
        #[derive(Serialize)]
        #[allow(clippy::enum_variant_names)]
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
    motor_speed: f32,
    motor_torque: f32,
    max_motor_torque: f32,
    motor_direction: MotorRotateDirection,
    motor_state: MotorState,
    mcu_main_state: MCUMainState,
    mcu_work_mode: MCUWorkMode,
    mcu_voltage: f32,
    mcu_current: f32,
    motor_temp: i32,
    mcu_temp: i32,
    dc_main_wire_over_volt_fault: bool,
    dc_main_wire_over_curr_fault: bool,
    motor_over_spd_fault: bool,
    motor_phase_curr_fault: bool,
    motor_stall_fault: bool,
    mcu_warning_level: MCUWarningLevel,
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
            if packet.len() != 58 {
                println!("Invalid packet length {}: {:?}", packet.len(), packet);
                continue;
            }
            send_message(TelemetryData {
                apps_travel: f32::from_le_bytes(packet[0..4].try_into().unwrap()),
                motor_speed: f32::from_le_bytes(packet[4..8].try_into().unwrap()),
                motor_torque: f32::from_le_bytes(packet[8..12].try_into().unwrap()),
                max_motor_torque: f32::from_le_bytes(packet[12..16].try_into().unwrap()),
                motor_direction: MotorRotateDirection::from_byte(packet[16]),
                motor_state: MotorState::from_byte(packet[17]),
                mcu_main_state: MCUMainState::from_byte(packet[18]),
                mcu_work_mode: MCUWorkMode::from_byte(packet[19]),
                mcu_voltage: f32::from_le_bytes(packet[20..24].try_into().unwrap()),
                mcu_current: f32::from_le_bytes(packet[24..28].try_into().unwrap()),
                motor_temp: i32::from_le_bytes(packet[28..32].try_into().unwrap()),
                mcu_temp: i32::from_le_bytes(packet[32..36].try_into().unwrap()),
                dc_main_wire_over_volt_fault: parse_bool(packet[36]),
                dc_main_wire_over_curr_fault: parse_bool(packet[37]),
                motor_over_spd_fault: parse_bool(packet[38]),
                motor_phase_curr_fault: parse_bool(packet[39]),
                motor_stall_fault: parse_bool(packet[40]),
                mcu_warning_level: MCUWarningLevel::from_byte(packet[41]),
                debug_0: f32::from_le_bytes(packet[42..46].try_into().unwrap()),
                debug_1: f32::from_le_bytes(packet[46..50].try_into().unwrap()),
                debug_2: f32::from_le_bytes(packet[50..54].try_into().unwrap()),
                debug_3: f32::from_le_bytes(packet[54..58].try_into().unwrap()),
            })
            .await;
        }
    }
}
