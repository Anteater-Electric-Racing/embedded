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
use rand::{rng, rngs::SmallRng, Rng, SeedableRng};
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
                        warn!(
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

impl Default for TelemetryData {
    fn default() -> Self {
        Self {
            apps_travel: 0.0,
            motor_speed: 0.0,
            motor_torque: 0.0,
            max_motor_torque: 0.0,
            motor_direction: MotorRotateDirection::DirectionStandby,
            motor_state: MotorState::MotorStateIdle,
            mcu_main_state: MCUMainState::StateStandby,
            mcu_work_mode: MCUWorkMode::WorkModeStandby,
            mcu_voltage: 0.0,
            mcu_current: 0.0,
            motor_temp: 0,
            mcu_temp: 0,
            dc_main_wire_over_volt_fault: false,
            dc_main_wire_over_curr_fault: false,
            motor_over_spd_fault: false,
            motor_phase_curr_fault: false,
            motor_stall_fault: false,
            mcu_warning_level: MCUWarningLevel::ErrorNone,
            fault_map: 0,
            over_current_fault: false,
            under_voltage_fault: false,
            over_temperature_fault: false,
            apps_fault: false,
            bse_fault: false,
            bpps_fault: false,
            apps_break_plausibility_fault: false,
            low_battery_voltage_fault: false,
        }
    }
}

impl TelemetryData {
    /// Parses a CAN_PACKET_SIZE-byte CAN packet into a [`TelemetryData`] struct.
    ///
    /// Returns `Err` if the packet length is wrong or any enum byte is invalid.
    pub fn from_bytes(bytes: &[u8]) -> Result<Self, String> {
        if bytes.len() != CAN_PACKET_SIZE {
            return Err(format!(
                "Expected {} bytes, got {}",
                CAN_PACKET_SIZE,
                bytes.len()
            ));
        }
        print!("Raw bytes: [");
        for (i, byte) in bytes.iter().enumerate() {
            if i > 0 {
                print!(", ");
            }
            print!("{:#04x}", byte);
        }
        println!("]");
        let fault: u32 = u32::from_be_bytes(bytes[42..46].try_into().unwrap());

        Ok(TelemetryData {
            apps_travel: f32::from_le_bytes(bytes[0..4].try_into().unwrap()),
            motor_speed: f32::from_le_bytes(bytes[4..8].try_into().unwrap()),
            motor_torque: f32::from_le_bytes(bytes[8..12].try_into().unwrap()),
            max_motor_torque: f32::from_le_bytes(bytes[12..16].try_into().unwrap()),
            motor_direction: MotorRotateDirection::from_byte(bytes[16])
                .ok_or_else(|| format!("Invalid motor_direction byte: {}", bytes[16]))?,
            motor_state: MotorState::from_byte(bytes[17])
                .ok_or_else(|| format!("Invalid motor_state byte: {}", bytes[17]))?,
            mcu_main_state: MCUMainState::from_byte(bytes[18])
                .ok_or_else(|| format!("Invalid mcu_main_state byte: {}", bytes[18]))?,
            mcu_work_mode: MCUWorkMode::from_byte(bytes[19])
                .ok_or_else(|| format!("Invalid mcu_work_mode byte: {}", bytes[19]))?,
            mcu_voltage: f32::from_le_bytes(bytes[20..24].try_into().unwrap()),
            mcu_current: f32::from_le_bytes(bytes[24..28].try_into().unwrap()),
            motor_temp: i32::from_le_bytes(bytes[28..32].try_into().unwrap()),
            mcu_temp: i32::from_le_bytes(bytes[32..36].try_into().unwrap()),
            dc_main_wire_over_volt_fault: bytes[36] != 0,
            dc_main_wire_over_curr_fault: bytes[37] != 0,
            motor_over_spd_fault: bytes[38] != 0,
            motor_phase_curr_fault: bytes[39] != 0,
            motor_stall_fault: bytes[40] != 0,
            mcu_warning_level: MCUWarningLevel::from_byte(bytes[41])
                .ok_or_else(|| format!("Invalid mcu_warning_level byte: {}", bytes[41]))?,
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
    pub fn to_bytes(&self) -> [u8; CAN_PACKET_SIZE] {
        let mut buf = [0u8; CAN_PACKET_SIZE];
        buf[0..4].copy_from_slice(&self.apps_travel.to_le_bytes());
        buf[4..8].copy_from_slice(&self.motor_speed.to_le_bytes());
        buf[8..12].copy_from_slice(&self.motor_torque.to_le_bytes());
        buf[12..16].copy_from_slice(&self.max_motor_torque.to_le_bytes());
        buf[16] = self.motor_direction as u8;
        buf[17] = self.motor_state as u8;
        buf[18] = self.mcu_main_state as u8;
        buf[19] = self.mcu_work_mode as u8;
        buf[20..24].copy_from_slice(&self.mcu_voltage.to_le_bytes());
        buf[24..28].copy_from_slice(&self.mcu_current.to_le_bytes());
        buf[28..32].copy_from_slice(&self.motor_temp.to_le_bytes());
        buf[32..36].copy_from_slice(&self.mcu_temp.to_le_bytes());
        buf[36] = self.dc_main_wire_over_volt_fault as u8;
        buf[37] = self.dc_main_wire_over_curr_fault as u8;
        buf[38] = self.motor_over_spd_fault as u8;
        buf[39] = self.motor_phase_curr_fault as u8;
        buf[40] = self.motor_stall_fault as u8;
        buf[41] = self.mcu_warning_level as u8;
        buf[42..46].copy_from_slice(&self.fault_map.to_le_bytes());
        buf
    }

    pub fn from_random(seed: u64) -> Self {
        let mut rng: SmallRng = SmallRng::seed_from_u64(seed);
        let mut raw = [0u8; CAN_PACKET_SIZE];
        rng.fill_bytes(&mut raw);
        if let Ok(random) = TelemetryData::from_bytes(&raw) {
            random
        } else {
            warn!(
                "Generated random bytes did not parse into valid TelemetryData, returning default"
            );
            TelemetryData::default()
        }
    }
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
    let mut tick: u64 = 0;

    loop {
        let synthetic = TelemetryData::from_random(0);

        send_message(synthetic).await;
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

    let payload = TelemetryData::to_bytes(data);
    socket.write_packet(&payload).await?;

    info!(bytes = payload.len(), "TelemetryData sent over ISO-TP");
    Ok(())
}

/// Verifies that [`parse_telemetry`] round-trips through
/// [`telemetry_to_raw_bytes`] without any CAN hardware.
#[test]
fn test_parse_telemetry_roundtrip() {
    let original = TelemetryData::from_random(0);

    let raw = TelemetryData::to_bytes(&original);
    let parsed = TelemetryData::from_bytes(&raw).expect("parse_telemetry failed");
    assert_eq!(original, parsed);
}

/// Rejects a packet that is too short.
#[test]
fn test_parse_telemetry_bad_length() {
    let short = [0u8; 10];
    assert!(TelemetryData::from_bytes(&short).is_err());
}

/// Rejects a packet containing an invalid enum byte.
#[test]
fn test_parse_telemetry_invalid_enum() {
    let mut raw = [0u8; CAN_PACKET_SIZE];
    // motor_direction at byte 16 — set to an invalid discriminant
    raw[16] = 200;
    assert!(TelemetryData::from_bytes(&raw).is_err());
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
    let data = TelemetryData::from_random(0);

    send_telemetry_over_isotp(&data).await?;
    Ok(())
}
