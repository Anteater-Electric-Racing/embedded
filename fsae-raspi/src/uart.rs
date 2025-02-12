use chrono::{DateTime, Utc};
use influxdb::{Client, InfluxDbWriteable};
use serde::Serialize;
use tokio::io::{AsyncBufReadExt, BufReader};
use tokio_serial::SerialPortBuilderExt;

use crate::send::{Sender, Reading};

// constants
const SERIAL_PORT: &str = "/dev/ttyACM0";
const SERIAL_BAUD_RATE: u32 = 9600;
const SIZE: usize = 4;

// UART reading struct
#[derive(InfluxDbWriteable, Serialize)]
struct UARTReading {
    time: DateTime<Utc>,
    brake_a: u16,
    brake_b: u16,
    shock_a: u16,
    shock_b: u16,
}

// make UARTReading sendable
impl Reading for UARTReading {
    fn topic() -> &'static str {
        "uart_reading"
    }
}

pub async fn read_uart() {
    let sender = Sender::new();

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

            if parts.len() != SIZE {
                eprintln!("Invalid number of parts: {:?}", parts);
                continue;
            }

            let (Ok(brake_a), Ok(brake_b), Ok(shock_a), Ok(shock_b)) = (
                parts[0].parse::<u16>(),
                parts[1].parse::<u16>(),
                parts[2].parse::<u16>(),
                parts[3].parse::<u16>(),
            ) else {
                eprintln!("Failed to parse integers from parts: {:?}", parts);
                continue;
            };

            let reading = UARTReading {
                time: Utc::now(),
                brake_a,
                brake_b,
                shock_a,
                shock_b,
            };

            sender.send_message(reading).await;
        }
    }
}
