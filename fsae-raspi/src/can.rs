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
use deku::prelude::*;
use serde::{Deserialize, Serialize};
use serde_repr::{Deserialize_repr, Serialize_repr};
use std::time::Duration;
use tokio::time::sleep;
use tokio_socketcan_isotp::{IsoTpSocket, StandardId};
use tracing::{error, info, warn};

const CAN_INTERFACE: &str = "can0";
const CAN_SRC_ID: u16 = 0x666;
const CAN_DST_ID: u16 = 0x777;

#[derive(
    Default,
    Debug,
    Deserialize_repr,
    Serialize_repr,
    PartialEq,
    Clone,
    Copy,
    DekuRead,
    DekuWrite,
    DekuSize,
)]
#[deku(ctx = "endian: deku::ctx::Endian")]
#[deku(id_type = "u8")]
#[repr(u8)]
pub enum MotorState {
    #[deku(id = 0)]
    #[default]
    MotorStateOff,
    #[deku(id = 1)]
    MotorStatePrecharging,
    #[deku(id = 2)]
    MotorStateIdle,
    #[deku(id = 3)]
    MotorStateDriving,
    #[deku(id = 4)]
    MotorStateFault,
}

#[derive(
    Default,
    Debug,
    Deserialize_repr,
    Serialize_repr,
    PartialEq,
    Clone,
    Copy,
    DekuRead,
    DekuWrite,
    DekuSize,
)]
#[deku(ctx = "endian: deku::ctx::Endian")]
#[deku(id_type = "u8")]
#[repr(u8)]
pub enum MotorRotateDirection {
    #[deku(id = 0)]
    #[default]
    DirectionStandby,
    #[deku(id = 1)]
    DirectionForward,
    #[deku(id = 2)]
    DirectionBackward,
    #[deku(id = 3)]
    DirectionError,
}

#[derive(
    Default,
    Debug,
    Deserialize_repr,
    Serialize_repr,
    PartialEq,
    Clone,
    Copy,
    DekuRead,
    DekuWrite,
    DekuSize,
)]
#[deku(ctx = "endian: deku::ctx::Endian")]
#[deku(id_type = "u8")]
#[repr(u8)]
pub enum MCUMainState {
    #[deku(id = 0)]
    #[default]
    StateStandby,
    #[deku(id = 1)]
    StatePrecharge,
    #[deku(id = 2)]
    StatePowerReady,
    #[deku(id = 3)]
    StateRun,
    #[deku(id = 4)]
    StatePowerOff,
}

#[derive(
    Default,
    Debug,
    Deserialize_repr,
    Serialize_repr,
    PartialEq,
    Clone,
    Copy,
    DekuRead,
    DekuWrite,
    DekuSize,
)]
#[deku(ctx = "endian: deku::ctx::Endian")]
#[deku(id_type = "u8")]
#[repr(u8)]
pub enum MCUWorkMode {
    #[deku(id = 0)]
    #[default]
    WorkModeStandby,
    #[deku(id = 1)]
    WorkModeTorque,
    #[deku(id = 2)]
    WorkModeSpeed,
}

#[derive(
    Default,
    Debug,
    Deserialize_repr,
    Serialize_repr,
    PartialEq,
    Clone,
    Copy,
    DekuRead,
    DekuWrite,
    DekuSize,
)]
#[deku(ctx = "endian: deku::ctx::Endian")]
#[deku(id_type = "u8")]
#[repr(u8)]
pub enum MCUWarningLevel {
    #[deku(id = 0)]
    #[default]
    ErrorNone,
    #[deku(id = 1)]
    ErrorLow,
    #[deku(id = 2)]
    ErrorMedium,
    #[deku(id = 3)]
    ErrorHigh,
}

/// Telemetry data record produced by the motor controller.
///
/// Parsed from the CAN_PACKET_SIZE-byte ISO-TP frame received over CAN.
/// Contains driver inputs, motor state information, controller status,
/// temperatures, electrical measurements, fault flags, and debug channels.
#[derive(
    Serialize, Deserialize, Default, Debug, Clone, PartialEq, DekuRead, DekuWrite, DekuSize,
)]
#[deku(endian = "little")]
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
    #[deku(bits = 1)]
    pub over_current: bool,
    #[deku(bits = 1)]
    pub under_voltage: bool,
    #[deku(bits = 1)]
    pub over_temperature: bool,
    #[deku(bits = 1)]
    pub apps: bool,
    #[deku(bits = 1)]
    pub bse: bool,
    #[deku(bits = 1)]
    pub bpps: bool,
    #[deku(bits = 1)]
    pub apps_brake_plaus: bool,
    #[deku(bits = 1, pad_bits_after = "24")]
    pub low_battery_voltage: bool,
}

impl Reading for TelemetryData {
    fn topic() -> &'static str {
        "telemetry"
    }
}

/// Reads ISO-TP packets from `can0` in a loop, parses each into
/// [`TelemetryData`], and forwards via [`send_message`].
///
/// Retries socket creation on failure; logs malformed packets.
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
            match TelemetryData::from_bytes((packet.as_ref(), 0)) {
                Ok(((remaining, _), _)) if !remaining.is_empty() => {
                    warn!("Telemetry packet has {} trailing bytes", remaining.len(),);
                }
                Ok((_, data)) => send_message(data).await,
                Err(e) => warn!(error = %e, "Malformed telemetry packet"),
            }
        }
    }
}

/// Generates synthetic telemetry (debug builds only).
async fn read_can_synthetic() {
    use std::time::Instant;

    let mut count: u64 = 0;
    let mut last = Instant::now();

    let mut interval = tokio::time::interval(Duration::from_millis(1));
    loop {
        interval.tick().await;
        send_message(TelemetryData::default()).await;
        count += 1;

        let elapsed = last.elapsed();
        if elapsed >= Duration::from_secs(1) {
            info!("{:.0} msg/s", count as f64 / elapsed.as_secs_f64());
            count = 0;
            last = Instant::now();
        }
    }
}

/// Entry point: dispatches to the hardware or synthetic reader depending
/// on the build profile.
pub async fn read_can() {
    if cfg!(feature = "synthetic") {
        read_can_synthetic().await;
    } else {
        read_can_hardware().await;
    }
}
