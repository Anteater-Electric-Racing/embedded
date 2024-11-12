use crate::influx::{INFLUXDB_DATABASE, INFLUXDB_URL};
use chrono::{DateTime, Utc};
use futures_util::StreamExt;
use influxdb::{Client, InfluxDbWriteable};
use socketcan::{tokio::CanSocket, EmbeddedFrame, ExtendedId, Id, StandardId};
use std::any::type_name;
use tokio;

// constants
const CAN_INTERFACE: &str = "can0";

// this sets up a trait that contains necessary information for all CAN messages that follow
pub trait CanReading: InfluxDbWriteable {
    // a function to get the expected ID of a CAN message
    fn id() -> Id;
    // a function to construct a CAN reading from the raw data
    fn construct(data: &[u8]) -> Self;
    // the expected size of the raw data
    const SIZE: usize;
}

// Now we list out all possible CAN messages
#[derive(InfluxDbWriteable)]
struct BMSReading1 {
    time: DateTime<Utc>,
    current: i16,
    inst_voltage: i16,
}

impl CanReading for BMSReading1 {
    fn id() -> Id {
        Id::Standard(StandardId::new(0x03B).unwrap())
    }
    fn construct(data: &[u8]) -> Self {
        BMSReading1 {
            time: Utc::now(),
            current: i16::from_be_bytes([data[0], data[1]]),
            inst_voltage: i16::from_be_bytes([data[2], data[3]]),
        }
    }
    const SIZE: usize = 4;
}

#[derive(InfluxDbWriteable)]
struct BMSReading2 {
    time: DateTime<Utc>,
    dlc: u8,
    ccl: u8,
    simulated_soc: u8,
    high_temp: u8,
    low_temp: u8,
}

impl CanReading for BMSReading2 {
    fn id() -> Id {
        Id::Standard(StandardId::new(0x3CB).unwrap())
    }
    fn construct(data: &[u8]) -> Self {
        BMSReading2 {
            time: Utc::now(),
            dlc: data[0],
            ccl: data[1],
            simulated_soc: data[2],
            high_temp: data[3],
            low_temp: data[4],
        }
    }
    const SIZE: usize = 5;
}

#[derive(InfluxDbWriteable)]
struct BMSReading3 {
    time: DateTime<Utc>,
    relay_state: u8,
    soc: u8,
    resistance: i16,
    open_voltage: i16,
    amphours: u8,
    pack_health: u8,
}

impl CanReading for BMSReading3 {
    fn id() -> Id {
        Id::Standard(StandardId::new(0x6B2).unwrap())
    }
    fn construct(data: &[u8]) -> Self {
        BMSReading3 {
            time: Utc::now(),
            relay_state: data[0],
            soc: data[1],
            resistance: i16::from_be_bytes([data[2], data[3]]),
            open_voltage: i16::from_be_bytes([data[4], data[5]]),
            amphours: data[6],
            pack_health: data[7],
        }
    }
    const SIZE: usize = 8;
}

#[derive(InfluxDbWriteable)]
struct LeftESCReading1 {
    time: DateTime<Utc>,
    speed_rpm: u16,
    motor_current: u16,
    battery_voltage: u16,
    error_code: u16,
}

impl CanReading for LeftESCReading1 {
    fn id() -> Id {
        Id::Extended(ExtendedId::new(0x0CF11E06).unwrap())
    }
    fn construct(data: &[u8]) -> Self {
        LeftESCReading1 {
            time: Utc::now(),
            speed_rpm: u16::from_le_bytes([data[0], data[1]]),
            motor_current: u16::from_le_bytes([data[2], data[3]]),
            battery_voltage: u16::from_le_bytes([data[4], data[5]]),
            error_code: u16::from_be_bytes([data[6], data[7]]),
        }
    }
    const SIZE: usize = 8;
}

#[derive(InfluxDbWriteable)]
struct LeftESCReading2 {
    time: DateTime<Utc>,
    throttle_signal: u8,
    controller_temp: i8,
    motor_temp: i8,
    controller_status: u8,
    switch_status: u8,
}

impl CanReading for LeftESCReading2 {
    fn id() -> Id {
        Id::Extended(ExtendedId::new(0x0CF11F06).unwrap())
    }
    fn construct(data: &[u8]) -> Self {
        LeftESCReading2 {
            time: Utc::now(),
            throttle_signal: data[0],
            controller_temp: data[1] as i8 - 40,
            motor_temp: data[2] as i8 - 30,
            controller_status: data[5],
            switch_status: data[6],
        }
    }
    const SIZE: usize = 8;
}

#[derive(InfluxDbWriteable)]
struct RightESCReading1 {
    time: DateTime<Utc>,
    speed_rpm: u16,
    motor_current: u16,
    battery_voltage: u16,
    error_code: u16,
}

impl CanReading for RightESCReading1 {
    fn id() -> Id {
        Id::Extended(ExtendedId::new(0x0CF11E05).unwrap())
    }
    fn construct(data: &[u8]) -> Self {
        RightESCReading1 {
            time: Utc::now(),
            speed_rpm: u16::from_le_bytes([data[0], data[1]]),
            motor_current: u16::from_le_bytes([data[2], data[3]]),
            battery_voltage: u16::from_le_bytes([data[4], data[5]]),
            error_code: u16::from_be_bytes([data[6], data[7]]),
        }
    }
    const SIZE: usize = 8;
}

#[derive(InfluxDbWriteable)]
struct RightESCReading2 {
    time: DateTime<Utc>,
    throttle_signal: u8,
    controller_temp: i8,
    motor_temp: i8,
    controller_status: u8,
    switch_status: u8,
}

impl CanReading for RightESCReading2 {
    fn id() -> Id {
        Id::Extended(ExtendedId::new(0x0CF11F05).unwrap())
    }
    fn construct(data: &[u8]) -> Self {
        RightESCReading2 {
            time: Utc::now(),
            throttle_signal: data[0],
            controller_temp: data[1] as i8 - 40,
            motor_temp: data[2] as i8 - 30,
            controller_status: data[5],
            switch_status: data[6],
        }
    }
    const SIZE: usize = 8;
}

// this checks a reading against a specific CAN message, and sends to influx if it matches
async fn check_message<T: CanReading + Send + 'static>(client: &Client, id: Id, data: &[u8]) {
    if T::id() == id {
        if data.len() != T::SIZE {
            eprintln!(
                "Invalid data length for {}: {}",
                type_name::<T>(),
                data.len()
            );
            return;
        }

        let reading = T::construct(data);

        let client = client.clone();
        tokio::spawn(async move {
            if let Err(e) = client.query(reading.into_query(type_name::<T>())).await {
                eprintln!("Failed to write to InfluxDB: {}", e);
            }
        });
    }
}

// finds all new CAN messages and sends to a check_message function for every possible CAN message
pub async fn read_can() {
    let client = Client::new(INFLUXDB_URL, INFLUXDB_DATABASE);

    loop {
        let Ok(mut sock) = CanSocket::open(CAN_INTERFACE) else {
            eprintln!("Failed to open CAN socket, retrying...");
            tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;
            continue;
        };
        while let Some(Ok(frame)) = sock.next().await {
            let data = frame.data();
            let id = frame.id();

            check_message::<BMSReading1>(&client, id, data).await;
            check_message::<BMSReading2>(&client, id, data).await;
            check_message::<BMSReading3>(&client, id, data).await;
            check_message::<LeftESCReading1>(&client, id, data).await;
            check_message::<LeftESCReading2>(&client, id, data).await;
            check_message::<RightESCReading1>(&client, id, data).await;
            check_message::<RightESCReading2>(&client, id, data).await;
        }
    }
}
