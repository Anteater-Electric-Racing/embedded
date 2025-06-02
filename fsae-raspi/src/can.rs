use crate::send::{send_message, Reading};
use chrono::{DateTime, Utc};
use influxdb::InfluxDbWriteable;
use serde::Serialize;
use tokio_socketcan_isotp::{IsoTpSocket, StandardId};

const CAN_INTERFACE: &str = "can0";

#[derive(Serialize, InfluxDbWriteable)]
struct TelemetryData {
    time: DateTime<Utc>,
    apps_travel: f32,
    bse_front_psi: f32,
    bse_rear_psi: f32,
    accumulator_voltage: f32,
    accumulator_temp_f: f32,
    motor_speed: f32,
    motor_torque: f32,
    max_motor_torque: f32,
    max_motor_brake_torque: f32,
    motor_direction: u8,
    mcu_main_state: u8,
    mcu_work_mode: u8,
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
    mcu_warning_level: u8,
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
            continue;
        };

        while let Ok(packet) = socket.read_packet().await {
            if packet.len() != 128 {
                println!("Invalid packet length: {}", packet.len());
                continue;
            }
            send_message(TelemetryData {
                time: Utc::now(),
                apps_travel: f32::from_le_bytes(packet[0..4].try_into().unwrap()),
                bse_front_psi: f32::from_le_bytes(packet[4..8].try_into().unwrap()),
                bse_rear_psi: f32::from_le_bytes(packet[8..12].try_into().unwrap()),
                accumulator_voltage: f32::from_le_bytes(packet[12..16].try_into().unwrap()),
                accumulator_temp_f: f32::from_le_bytes(packet[16..20].try_into().unwrap()),
                motor_speed: f32::from_le_bytes(packet[20..24].try_into().unwrap()),
                motor_torque: f32::from_le_bytes(packet[24..28].try_into().unwrap()),
                max_motor_torque: f32::from_le_bytes(packet[28..32].try_into().unwrap()),
                max_motor_brake_torque: f32::from_le_bytes(packet[32..36].try_into().unwrap()),
                motor_direction: packet[36],
                mcu_main_state: packet[37],
                mcu_work_mode: packet[38],
                motor_temp: i32::from_le_bytes(packet[39..43].try_into().unwrap()),
                mcu_temp: i32::from_le_bytes(packet[43..47].try_into().unwrap()),
                dc_main_wire_over_volt_fault: packet[47] != 0,
                motor_phase_curr_fault: packet[48] != 0,
                mcu_over_hot_fault: packet[49] != 0,
                resolver_fault: packet[50] != 0,
                phase_curr_sensor_fault: packet[51] != 0,
                motor_over_spd_fault: packet[52] != 0,
                drv_motor_over_hot_fault: packet[53] != 0,
                dc_main_wire_over_curr_fault: packet[54] != 0,
                drv_motor_over_cool_fault: packet[55] != 0,
                mcu_motor_system_state: packet[56] != 0,
                mcu_temp_sensor_state: packet[57] != 0,
                motor_temp_sensor_state: packet[58] != 0,
                dc_volt_sensor_state: packet[59] != 0,
                dc_low_volt_warning: packet[60] != 0,
                mcu_12v_low_volt_warning: packet[61] != 0,
                motor_stall_fault: packet[62] != 0,
                motor_open_phase_fault: packet[63] != 0,
                mcu_warning_level: packet[64],
                mcu_voltage: f32::from_le_bytes(packet[65..69].try_into().unwrap()),
                mcu_current: f32::from_le_bytes(packet[69..73].try_into().unwrap()),
                motor_phase_curr: f32::from_le_bytes(packet[73..77].try_into().unwrap()),
                debug_0: f32::from_le_bytes(packet[77..81].try_into().unwrap()),
                debug_1: f32::from_le_bytes(packet[81..85].try_into().unwrap()),
                debug_2: f32::from_le_bytes(packet[85..89].try_into().unwrap()),
                debug_3: f32::from_le_bytes(packet[89..93].try_into().unwrap()),
            })
            .await;
        }
    }
}
