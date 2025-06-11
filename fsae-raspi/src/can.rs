use crate::send::{send_message, Reading};
use chrono::{DateTime, Utc};
use influxdb::InfluxDbWriteable;
use serde::Serialize;
use tokio_socketcan_isotp::{IsoTpSocket, StandardId};

const CAN_INTERFACE: &str = "can0";

#[derive(Serialize, InfluxDbWriteable)]
struct CANMessage {
    time: DateTime<Utc>,
    accumulator_voltage: f32,
    accumulator_temp: f32,
    tractive_system_voltage: f32,
    adc_0_read_0: u16,
    adc_0_read_1: u16,
    adc_0_read_2: u16,
    adc_0_read_3: u16,
    adc_0_read_4: u16,
    adc_0_read_5: u16,
    adc_0_read_6: u16,
    adc_0_read_7: u16,
    adc_1_read_0: u16,
    adc_1_read_1: u16,
    adc_1_read_2: u16,
    adc_1_read_3: u16,
    adc_1_read_4: u16,
    adc_1_read_5: u16,
    adc_1_read_6: u16,
    adc_1_read_7: u16,
}

impl Reading for CANMessage {
    fn topic() -> &'static str {
        "can"
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
            if packet.len() != 44 {
                println!("Invalid packet length: {}", packet.len());
                continue;
            }
            send_message(CANMessage {
                time: Utc::now(),
                accumulator_voltage: f32::from_le_bytes(packet[0..4].try_into().unwrap()),
                accumulator_temp: f32::from_le_bytes(packet[4..8].try_into().unwrap()),
                tractive_system_voltage: f32::from_le_bytes(packet[8..12].try_into().unwrap()),
                adc_0_read_0: u16::from_le_bytes(packet[12..14].try_into().unwrap()),
                adc_0_read_1: u16::from_le_bytes(packet[14..16].try_into().unwrap()),
                adc_0_read_2: u16::from_le_bytes(packet[16..18].try_into().unwrap()),
                adc_0_read_3: u16::from_le_bytes(packet[18..20].try_into().unwrap()),
                adc_0_read_4: u16::from_le_bytes(packet[20..22].try_into().unwrap()),
                adc_0_read_5: u16::from_le_bytes(packet[22..24].try_into().unwrap()),
                adc_0_read_6: u16::from_le_bytes(packet[24..26].try_into().unwrap()),
                adc_0_read_7: u16::from_le_bytes(packet[26..28].try_into().unwrap()),
                adc_1_read_0: u16::from_le_bytes(packet[28..30].try_into().unwrap()),
                adc_1_read_1: u16::from_le_bytes(packet[30..32].try_into().unwrap()),
                adc_1_read_2: u16::from_le_bytes(packet[32..34].try_into().unwrap()),
                adc_1_read_3: u16::from_le_bytes(packet[34..36].try_into().unwrap()),
                adc_1_read_4: u16::from_le_bytes(packet[36..38].try_into().unwrap()),
                adc_1_read_5: u16::from_le_bytes(packet[38..40].try_into().unwrap()),
                adc_1_read_6: u16::from_le_bytes(packet[40..42].try_into().unwrap()),
                adc_1_read_7: u16::from_le_bytes(packet[42..44].try_into().unwrap()),
            })
            .await;
        }
    }
}
