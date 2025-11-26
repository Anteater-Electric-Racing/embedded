//! This module enables the recieving and decoding of telemetry
//! data from the motor controller over CAN ISO-TP and sets up test
//! functions for transmitting test packets over vCAN.
//!
//! Module includes:
//!
//! - ISO-TP socket setup for receiving and sending frames
//! - Parsing of the MCU telemetry packet into typed data
//! - Definitions of all enums representing MCU and motor state machines
//! - 'TelemetryData' structure, which contains the fully decoded packet
//! - Test function to send and verify dummy telemetry over vcan0
//!
//! Code runs use relies on 'can0', while tests can run against vcan0
//! using the virtual can network setup on Github actions workflow (test.yml)

use crate::send::{send_message, Reading};
#[cfg(test)]
use bincode;
use serde::{Deserialize, Serialize};
#[cfg(test)]
use std::error::Error;
use std::time::Duration;
use tokio::time::sleep;
use tokio_socketcan_isotp::{IsoTpSocket, StandardId};

const CAN_INTERFACE: &str = "can0";

macro_rules! define_enum {
    ($name:ident, $($variant:ident = $value:expr),*) => {
        #[derive(Serialize, Deserialize, PartialEq, Clone)]
        #[allow(clippy::enum_variant_names)]
        pub enum $name {
            $($variant = $value),*
        }

        impl $name {
            pub fn from_byte(byte: u8) -> Self {
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

/// Telemetry data record produced by the motor controller.
///
/// Structure from the ISO-TP frame received over
/// CAN. It contains driver inputs, motor state information, controller status,
/// temperatures, electrical measurements, fault flags, and debug channels.
///
/// Fields match up with the telemetryData packet in
/// test_send_telemetry_over_isotp().
#[derive(Serialize, Deserialize, PartialEq, Clone)]
pub struct TelemetryData {
    pub apps_travel: f32,
    pub motor_speed: f32,
    pub motor_torque: f32,
    pub max_motor_torque: f32,
    pub motor_direction: MotorRotateDirection,
    pub motor_state: MotorState,
    pub mcu_main_state: MCUMainState,
    pub mcu_work_mode: MCUWorkMode,
    pub mcu_voltage: f32,
    pub mcu_current: f32,
    pub motor_temp: i32,
    pub mcu_temp: i32,
    pub dc_main_wire_over_volt_fault: bool,
    pub dc_main_wire_over_curr_fault: bool,
    pub motor_over_spd_fault: bool,
    pub motor_phase_curr_fault: bool,
    pub motor_stall_fault: bool,
    pub mcu_warning_level: MCUWarningLevel,
    pub debug_0: f32,
    pub debug_1: f32,
    pub debug_2: f32,
    pub debug_3: f32,
}

impl Reading for TelemetryData {
    fn topic() -> &'static str {
        "telemetry"
    }
}

fn parse_bool(byte: u8) -> bool {
    byte != 0
}

/// Continuously reads ISO-TP packets from 'can0', parses the MCU
/// telemetry frame, converts it into a TelemetryData struct, and forwards it
/// via 'send_message()'.
///
/// Loop retries socket creation on failure and logs malformed packets.
///
/// Expected packet length of 58 bytes.
pub async fn read_can() {
    loop {
        let socket = match IsoTpSocket::open(
            CAN_INTERFACE,
            StandardId::new(0x666).expect("Invalid src id"),
            StandardId::new(0x777).expect("Invalid src id"),
        ) {
            Ok(socket) => socket,
            Err(e) => {
                println!("Failed to open socket: {}", e);
                sleep(Duration::from_secs(1)).await;
                continue;
            }
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

/// Sends a 'TelemetryData' instance over ISO-TP on 'vcan0'.
///
/// This function is intended for testing and is only compiled in 'cfg(test)'
/// mode. The struct is serialized using bincode and written as a single
/// ISO-TP packet.
///
/// Requires a virtual CAN interface.
#[cfg(test)]
async fn send_telemetry_over_isotp(data: &TelemetryData) -> Result<(), Box<dyn Error>> {
    let socket = IsoTpSocket::open(
        "vcan0",
        StandardId::new(0x123).ok_or("Invalid source ID")?,
        StandardId::new(0x321).ok_or("Invalid destination ID")?,
    )?;

    let payload = bincode::serde::encode_to_vec(&data, bincode::config::legacy())?;

    socket.write_packet(&payload).await?;

    println!("TelemetryData sent over iso-tp ({} bytes)", payload.len());
    Ok(())
}

/// Test verifying ISO-TP transmission by sending and verifying dummy `TelemetryData` packet over ISO-TP
///
/// Requires a virtual CAN interface configured with:
/// ''' bash
/// sudo modprobe vcan
/// sudo ip link add dev vcan0 type vcan
/// sudo ip link set up vcan0
/// '''
/// ^^ vCAN commands automatically setup and verify
///    'test_send_telemetry_over_isotp' function
//     on test.yml Github actions workflow

#[tokio::test]
async fn test_send_telemetry_over_isotp() -> Result<(), Box<dyn Error>> {
    let data = TelemetryData {
        apps_travel: 72.5,
        motor_speed: 3200.0,
        motor_torque: 85.4,
        max_motor_torque: 120.0,
        motor_direction: MotorRotateDirection::DirectionForward,
        motor_state: MotorState::MotorStateDriving,
        mcu_main_state: MCUMainState::StateRun,
        mcu_work_mode: MCUWorkMode::WorkModeTorque,
        mcu_voltage: 13.8,
        mcu_current: 2.4,
        motor_temp: 75,
        mcu_temp: 68,
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

    send_telemetry_over_isotp(&data).await?;
    Ok(())
}
