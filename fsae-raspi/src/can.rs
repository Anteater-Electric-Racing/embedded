use std::time::Duration;
use crate::send::{send_message, Reading};
use chrono::{DateTime, Utc};
use serde::Serialize;
use tokio::time::sleep;
use tokio_socketcan_isotp::{IsoTpSocket, StandardId};

const CAN_INTERFACE: &str = "can0";

macro_rules! define_enum {
    ($name:ident, $($variant:ident = $value:expr => $text:expr),*) => {
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
            fn as_str(&self) -> &'static str {
                match self {
                    $(Self::$variant => $text),*
                }
            }
        }

        impl serde::Serialize for $name {
            fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
            where
                S: serde::Serializer,
            {
                serializer.serialize_str(self.as_str())
            }
        }
    };
}

define_enum!(MotorState,
    MotorStateOff = 0 => "Off",
    MotorStatePrecharging = 1 => "Precharging",
    MotorStateIdle = 2 => "Idle",
    MotorStateDriving = 3 => "Driving",
    MotorStateFault = 4 => "Fault"
);

define_enum!(MotorRotateDirection,
    DirectionStandby = 0 => "Standby",
    DirectionForward = 1 => "Forward",
    DirectionBackward = 2 => "Backward",
    DirectionError = 3 => "Error"
);

define_enum!(MCUMainState,
    StateStandby = 0 => "Standby",
    StatePrecharge = 1 => "Precharge",
    StatePowerReady = 2 => "PowerReady",
    StateRun = 3 => "Run",
    StatePowerOff = 4 => "PowerOff"
);

define_enum!(MCUWorkMode,
    WorkModeStandby = 0 => "Standby",
    WorkModeTorque = 1 => "Torque",
    WorkModeSpeed = 2 => "Speed"
);

define_enum!(MCUWarningLevel,
    ErrorNone = 0 => "None",
    ErrorLow = 1 => "Low",
    ErrorMedium = 2 => "Medium",
    ErrorHigh = 3 => "High"
);

#[derive(Serialize)]
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
}