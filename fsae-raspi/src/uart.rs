use crate::influx::{INFLUXDB_DATABASE, INFLUXDB_URL};
use chrono::{DateTime, Utc};
use influxdb::{Client, InfluxDbWriteable};
use std::any::type_name;
use tokio::io::{AsyncBufReadExt, BufReader};
use tokio_serial::SerialPortBuilderExt;

// constants
const SERIAL_PORT: &str = "/dev/ttyACM0";
const SERIAL_BAUD_RATE: u32 = 9600;

// UART reading struct
#[derive(InfluxDbWriteable)]
struct UARTReading {
    time: DateTime<Utc>,
    brake_a: u16,
    brake_b: u16,
    shock_a: u16,
    shock_b: u16,
}

// expected number of parts in a UART reading
impl UARTReading {
    const SIZE: usize = 4;
}

// check the UART reading and write to InfluxDB
async fn check_uart(client: &Client, parts: Vec<&str>) {
    if parts.len() != UARTReading::SIZE {
        eprintln!("Invalid number of parts: {:?}", parts);
        return;
    }
    let (Ok(brake_a), Ok(brake_b), Ok(shock_a), Ok(shock_b)) = (
        parts[0].parse::<u16>(),
        parts[1].parse::<u16>(),
        parts[2].parse::<u16>(),
        parts[3].parse::<u16>(),
    ) else {
        eprintln!("Failed to parse integers from parts: {:?}", parts);
        return;
    };
    let reading = UARTReading {
        time: Utc::now(),
        brake_a,
        brake_b,
        shock_a,
        shock_b,
    };

    let client = client.clone();
    tokio::spawn(async move {
        if let Err(e) = client
            .query(reading.into_query(type_name::<UARTReading>()))
            .await
        {
            eprintln!("Failed to write to InfluxDB: {}", e);
        }
    });
}

// read from the UART port and parse the data
pub async fn read_uart() {
    let client = Client::new(INFLUXDB_URL, INFLUXDB_DATABASE);

    if cfg!(not(debug_assertions)) {
        loop {
            let Ok(serial) = tokio_serial::new(SERIAL_PORT, SERIAL_BAUD_RATE).open_native_async()
            else {
                eprintln!("Failed to open serial port, retrying...");
                tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;
                continue;
            };

            let reader = BufReader::new(serial);
            let mut lines = reader.lines();

            while let Ok(Some(line)) = lines.next_line().await {
                let parts: Vec<&str> = line.trim().split_whitespace().collect();
                check_uart(&client, parts).await;
            }
        }
    }
}
