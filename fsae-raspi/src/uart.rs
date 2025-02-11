use crate::influx::{INFLUXDB_DATABASE, INFLUXDB_URL};
use chrono::{DateTime, Utc};
use influxdb::{Client, InfluxDbWriteable};
use std::any::type_name;
use tokio::io::{AsyncBufReadExt, BufReader};
use tokio_serial::SerialPortBuilderExt;

// constants
const SERIAL_PORT: &str = "/dev/ttyACM0";
const SERIAL_BAUD_RATE: u32 = 9600;
const SIZE: usize = 4;

// UART reading struct
#[derive(InfluxDbWriteable)]
struct UARTReading {
    time: DateTime<Utc>,
    brake_a: u16,
    brake_b: u16,
    shock_a: u16,
    shock_b: u16,
}

// check the UART reading and write to InfluxDB
async fn check_uart(client: &Client, parts: Vec<&str>) {
    if parts.len() != SIZE {
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
            .query(reading.into_query("uart_reading"))
            .await
        {
            eprintln!("Failed to write to InfluxDB: {}", e);
        }
    });
}

// read from the UART port and parse the data
pub async fn read_uart() {
    let client = Client::new(INFLUXDB_URL, INFLUXDB_DATABASE);

    loop {
        let serial = match tokio_serial::new(SERIAL_PORT, SERIAL_BAUD_RATE).open_native_async() {
            Ok(serial) => serial,
            Err(e) => {
                eprintln!("Failed to open serial port: {}", e);
                tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;
                continue;
            }
        };

        let mut lines = BufReader::new(serial).lines();

        while let Ok(Some(line)) = lines.next_line().await {
            let parts: Vec<&str> = line.split_whitespace().collect();
            check_uart(&client, parts).await;
        }
    }
}
