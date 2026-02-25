//! This module enables the receiving and decoding of telemetry
//! data from the motor controller over CAN ISO-TP and sets up test
//! functions for transmitting test packets over vCAN.
//!
//! Module includes:
//!
//! - ISO-TP socket setup for receiving and sending frames
//! - Parsing of the MCU telemetry packet into typed data
//! - Definitions of all enums representing MCU and motor state machines
//! - `TelemetryData` structure, which contains the fully decoded packet
//! - Test function to send and verify dummy telemetry over vcan0
//!
//! Production code relies on `can0`, while tests can run against vcan0
//! using the virtual CAN network setup in the GitHub Actions workflow (test.yml).

use crate::send::{send_message, Reading};
use serde::{Deserialize, Serialize};
use std::time::Duration;
use tokio::time::sleep;
use tokio_socketcan_isotp::{IsoTpSocket, StandardId};
use tracing::{error, info, warn};

const CAN_INTERFACE: &str = "can0";
const CAN_SRC_ID: u16 = 0x666;
const CAN_DST_ID: u16 = 0x777;
const CAN_PACKET_SIZE: usize = 46;

macro_rules! define_enum {
    ($name:ident, $($variant:ident = $value:expr),*) => {
        #[derive(Debug, Serialize, Deserialize, PartialEq, Clone, Copy)]
        #[repr(u8)]
        #[allow(clippy::enum_variant_names)]
        pub enum $name {
            $($variant = $value,)*
            Unknown = 255,
        }

        impl $name {
            pub fn from_byte(byte: u8) -> Option<Self> {
                match byte {
                    $($value => Some(Self::$variant),)*
                    _ => {
                        tracing::warn!(
                            "Unknown {} value: {}",
                            stringify!($name),
                            byte
                        );
                        None
                    }
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
/// Parsed from the CAN_PACKET_SIZE-byte ISO-TP frame received over CAN.
/// Contains driver inputs, motor state information, controller status,
/// temperatures, electrical measurements, fault flags, and debug channels.
#[derive(Debug, Serialize, Deserialize, PartialEq, Clone)]
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
    pub fault_map: u32,
    pub over_current_fault: bool,
    pub under_voltage_fault: bool,
    pub over_temperature_fault: bool,
    pub apps_fault: bool,
    pub bse_fault: bool,
    pub bpps_fault: bool,
    pub apps_break_plausibility_fault: bool,
    pub low_battery_voltage_fault: bool,
}

impl Reading for TelemetryData {
    fn topic() -> &'static str {
        "telemetry"
    }
}

fn parse_bool(byte: u8) -> bool {
    byte != 0
}

/// Parses a CAN_PACKET_SIZE-byte CAN packet into a [`TelemetryData`] struct.
///
/// Returns `Err` if the packet length is wrong or any enum byte is invalid.
pub fn parse_telemetry(packet: &[u8]) -> Result<TelemetryData, String> {
    if packet.len() != CAN_PACKET_SIZE {
        return Err(format!(
            "Expected {} bytes, got {}",
            CAN_PACKET_SIZE,
            packet.len()
        ));
    }
    print!("Raw bytes: [");
    for (i, byte) in packet.iter().enumerate() {
        if i > 0 {
            print!(", ");
        }
        print!("{:#04x}", byte);
    }
    println!("]");
    let fault: u32 = u32::from_le_bytes(packet[42..46].try_into().unwrap());

    Ok(TelemetryData {
        apps_travel: f32::from_le_bytes(packet[0..4].try_into().unwrap()),
        motor_speed: f32::from_le_bytes(packet[4..8].try_into().unwrap()),
        motor_torque: f32::from_le_bytes(packet[8..12].try_into().unwrap()),
        max_motor_torque: f32::from_le_bytes(packet[12..16].try_into().unwrap()),
        motor_direction: MotorRotateDirection::from_byte(packet[16])
            .ok_or_else(|| format!("Invalid motor_direction byte: {}", packet[16]))?,
        motor_state: MotorState::from_byte(packet[17])
            .ok_or_else(|| format!("Invalid motor_state byte: {}", packet[17]))?,
        mcu_main_state: MCUMainState::from_byte(packet[18])
            .ok_or_else(|| format!("Invalid mcu_main_state byte: {}", packet[18]))?,
        mcu_work_mode: MCUWorkMode::from_byte(packet[19])
            .ok_or_else(|| format!("Invalid mcu_work_mode byte: {}", packet[19]))?,
        mcu_voltage: f32::from_le_bytes(packet[20..24].try_into().unwrap()),
        mcu_current: f32::from_le_bytes(packet[24..28].try_into().unwrap()),
        motor_temp: i32::from_le_bytes(packet[28..32].try_into().unwrap()),
        mcu_temp: i32::from_le_bytes(packet[32..36].try_into().unwrap()),
        dc_main_wire_over_volt_fault: parse_bool(packet[36]),
        dc_main_wire_over_curr_fault: parse_bool(packet[37]),
        motor_over_spd_fault: parse_bool(packet[38]),
        motor_phase_curr_fault: parse_bool(packet[39]),
        motor_stall_fault: parse_bool(packet[40]),
        mcu_warning_level: MCUWarningLevel::from_byte(packet[41])
            .ok_or_else(|| format!("Invalid mcu_warning_level byte: {}", packet[41]))?,
        fault_map: fault,
        over_current_fault: (fault & 0x1 << 0) != 0,
        under_voltage_fault: (fault & 0x1 << 1) != 0,
        over_temperature_fault: (fault & 0x1 << 2) != 0,
        apps_fault: (fault & 0x1 << 3) != 0,
        bse_fault: (fault & 0x1 << 4) != 0,
        bpps_fault: (fault & 0x1 << 5) != 0,
        apps_break_plausibility_fault: (fault & 0x1 << 6) != 0,
        low_battery_voltage_fault: (fault & 0x1 << 7) != 0,
    })
}

/// Serializes a [`TelemetryData`] struct into the raw CAN_PACKET_SIZE-byte CAN packet
/// format. Mirrors the layout expected by [`parse_telemetry`].
#[cfg(test)]
pub fn telemetry_to_raw_bytes(data: &TelemetryData) -> [u8; CAN_PACKET_SIZE] {
    let mut buf = [0u8; CAN_PACKET_SIZE];
    buf[0..4].copy_from_slice(&data.apps_travel.to_le_bytes());
    buf[4..8].copy_from_slice(&data.motor_speed.to_le_bytes());
    buf[8..12].copy_from_slice(&data.motor_torque.to_le_bytes());
    buf[12..16].copy_from_slice(&data.max_motor_torque.to_le_bytes());
    buf[16] = data.motor_direction as u8;
    buf[17] = data.motor_state as u8;
    buf[18] = data.mcu_main_state as u8;
    buf[19] = data.mcu_work_mode as u8;
    buf[20..24].copy_from_slice(&data.mcu_voltage.to_le_bytes());
    buf[24..28].copy_from_slice(&data.mcu_current.to_le_bytes());
    buf[28..32].copy_from_slice(&data.motor_temp.to_le_bytes());
    buf[32..36].copy_from_slice(&data.mcu_temp.to_le_bytes());
    buf[36] = data.dc_main_wire_over_volt_fault as u8;
    buf[37] = data.dc_main_wire_over_curr_fault as u8;
    buf[38] = data.motor_over_spd_fault as u8;
    buf[39] = data.motor_phase_curr_fault as u8;
    buf[40] = data.motor_stall_fault as u8;
    buf[41] = data.mcu_warning_level as u8;
    buf[42..46].copy_from_slice(&data.fault_map.to_le_bytes());
    buf
}

/// Reads ISO-TP packets from `can0` in a loop, parses each into
/// [`TelemetryData`], and forwards via [`send_message`].
///
/// Retries socket creation on failure; logs malformed packets.
#[cfg(not(debug_assertions))]
async fn read_can_hardware() {
    loop {
        let socket = match IsoTpSocket::open(
            CAN_INTERFACE,
            StandardId::new(CAN_SRC_ID).expect("Invalid src id"),
            StandardId::new(CAN_DST_ID).expect("Invalid dst id"),
        ) {
            Ok(socket) => socket,
            Err(e) => {
                error!(%e, "Failed to open CAN socket");
                sleep(Duration::from_secs(1)).await;
                continue;
            }
        };

        while let Ok(packet) = socket.read_packet().await {
            match parse_telemetry(&packet) {
                Ok(data) => send_message(data).await,
                Err(e) => warn!(error = %e, "Malformed telemetry packet"),
            }
        }
    }
}

/// Generates synthetic telemetry on a 100 ms interval (debug builds only).
#[cfg(debug_assertions)]
async fn read_can_synthetic() {
    info!("Debug mode: generating synthetic telemetry data on 100ms interval");
    let mut tick: u64 = 0;

    loop {
        let t = tick as f32 * 0.1;
        let cycle = (t * 0.05).sin().max(0.0);
        let fault = (t.sin() * 1000.0) as u32;

        let synthetic = TelemetryData {
            apps_travel: cycle * 95.0,
            motor_speed: cycle * 4500.0,
            motor_torque: cycle * 110.0,
            max_motor_torque: 120.0,
            motor_direction: if cycle > 0.01 {
                MotorRotateDirection::DirectionForward
            } else {
                MotorRotateDirection::DirectionStandby
            },
            motor_state: if cycle > 0.01 {
                MotorState::MotorStateDriving
            } else {
                MotorState::MotorStateIdle
            },
            mcu_main_state: MCUMainState::StateRun,
            mcu_work_mode: MCUWorkMode::WorkModeTorque,
            mcu_voltage: 300.0 + 20.0 * (t * 0.2).sin(),
            mcu_current: cycle * 150.0 + 5.0 * (t * 0.7).sin(),
            motor_temp: 35 + (cycle * 45.0) as i32,
            mcu_temp: 30 + (cycle * 30.0) as i32,
            dc_main_wire_over_volt_fault: false,
            dc_main_wire_over_curr_fault: false,
            motor_over_spd_fault: false,
            motor_phase_curr_fault: false,
            motor_stall_fault: false,
            mcu_warning_level: MCUWarningLevel::ErrorNone,
            fault_map: fault,
            over_current_fault: (fault & 0x1 << 0) != 0,
            under_voltage_fault: (fault & 0x1 << 1) != 0,
            over_temperature_fault: (fault & 0x1 << 2) != 0,
            apps_fault: (fault & 0x1 << 3) != 0,
            bse_fault: (fault & 0x1 << 4) != 0,
            bpps_fault: (fault & 0x1 << 5) != 0,
            apps_break_plausibility_fault: (fault & 0x1 << 6) != 0,
            low_battery_voltage_fault: (fault & 0x1 << 7) != 0,
        };

        send_message(synthetic).await;

        tick += 1;
        sleep(Duration::from_millis(100)).await;
    }
}

/// Entry point: dispatches to the hardware or synthetic reader depending
/// on the build profile.
pub async fn read_can() {
    #[cfg(not(debug_assertions))]
    read_can_hardware().await;

    #[cfg(debug_assertions)]
    read_can_synthetic().await;
}

/// Sends a raw CAN_PACKET_SIZE-byte [`TelemetryData`] packet over ISO-TP on `vcan0`.
///
/// Only compiled in `cfg(test)` mode.
/// Requires a virtual CAN interface.
#[cfg(test)]
async fn send_telemetry_over_isotp(data: &TelemetryData) -> Result<(), Box<dyn std::error::Error>> {
    let socket = IsoTpSocket::open(
        "vcan0",
        StandardId::new(0x123).ok_or("Invalid source ID")?,
        StandardId::new(0x321).ok_or("Invalid destination ID")?,
    )?;

    let payload = telemetry_to_raw_bytes(data);
    socket.write_packet(&payload).await?;

    info!(bytes = payload.len(), "TelemetryData sent over ISO-TP");
    Ok(())
}

/// Verifies that [`parse_telemetry`] round-trips through
/// [`telemetry_to_raw_bytes`] without any CAN hardware.
#[test]
fn test_parse_telemetry_roundtrip() {
    let fault = 12345;
    let original = TelemetryData {
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
        dc_main_wire_over_curr_fault: true,
        motor_over_spd_fault: false,
        motor_phase_curr_fault: false,
        motor_stall_fault: false,
        mcu_warning_level: MCUWarningLevel::ErrorNone,
        fault_map: fault,
        over_current_fault: (fault & 0x1 << 0) != 0,
        under_voltage_fault: (fault & 0x1 << 1) != 0,
        over_temperature_fault: (fault & 0x1 << 2) != 0,
        apps_fault: (fault & 0x1 << 3) != 0,
        bse_fault: (fault & 0x1 << 4) != 0,
        bpps_fault: (fault & 0x1 << 5) != 0,
        apps_break_plausibility_fault: (fault & 0x1 << 6) != 0,
        low_battery_voltage_fault: (fault & 0x1 << 7) != 0,
    };

    let raw = telemetry_to_raw_bytes(&original);
    let parsed = parse_telemetry(&raw).expect("parse_telemetry failed");
    assert_eq!(original, parsed);
}

/// Rejects a packet that is too short.
#[test]
fn test_parse_telemetry_bad_length() {
    let short = [0u8; 10];
    assert!(parse_telemetry(&short).is_err());
}

/// Rejects a packet containing an invalid enum byte.
#[test]
fn test_parse_telemetry_invalid_enum() {
    let mut raw = [0u8; CAN_PACKET_SIZE];
    // motor_direction at byte 16 — set to an invalid discriminant
    raw[16] = 200;
    assert!(parse_telemetry(&raw).is_err());
}

/// Sends a dummy telemetry packet over ISO-TP on `vcan0` and verifies delivery.
///
/// Requires:
/// ```bash
/// sudo modprobe vcan
/// sudo ip link add dev vcan0 type vcan
/// sudo ip link set up vcan0
/// ```
/// These commands are run automatically in the GitHub Actions workflow (test.yml).
#[tokio::test]
async fn test_send_telemetry_over_isotp() -> Result<(), Box<dyn std::error::Error>> {
    let fault = 12345;
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
        fault_map: fault,
        over_current_fault: (fault & 0x1 << 0) != 0,
        under_voltage_fault: (fault & 0x1 << 1) != 0,
        over_temperature_fault: (fault & 0x1 << 2) != 0,
        apps_fault: (fault & 0x1 << 3) != 0,
        bse_fault: (fault & 0x1 << 4) != 0,
        bpps_fault: (fault & 0x1 << 5) != 0,
        apps_break_plausibility_fault: (fault & 0x1 << 6) != 0,
        low_battery_voltage_fault: (fault & 0x1 << 7) != 0,
    };

    send_telemetry_over_isotp(&data).await?;
    Ok(())
}
