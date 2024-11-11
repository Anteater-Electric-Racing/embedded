use chrono::{DateTime, Utc};
use fsae_macro::MessageSize;
use futures_util::StreamExt;
use socketcan::{tokio::CanSocket, Id, Result, StandardId, EmbeddedFrame};
use std::any::type_name;
use tokio;

const CAN_INTERFACE: &str = "can0";

#[derive(MessageSize, Debug)]
struct PackReading1 {
    time: DateTime<Utc>,
    current: i16,
    inst_voltage: i16,
}

impl CanReading for PackReading1 {
    fn id() -> Id {
        Id::Standard(StandardId::new(0x03B).unwrap())
    }
    fn construct(data: &[u8]) -> Self {
        PackReading1 {
            time: Utc::now(),
            current: i16::from_be_bytes([data[0], data[1]]),
            inst_voltage: i16::from_be_bytes([data[2], data[3]]),
        }
    }
}

#[derive(MessageSize, Debug)]
struct PackReading2 {
    time: DateTime<Utc>,
    dlc: u8,
    ccl: u8,
    simulated_soc: u8,
    high_temp: u8,
    low_temp: u8,
}

impl CanReading for PackReading2 {
    fn id() -> Id {
        Id::Standard(StandardId::new(0x3CB).unwrap())
    }
    fn construct(data: &[u8]) -> Self {
        PackReading2 {
            time: Utc::now(),
            dlc: data[0],
            ccl: data[1],
            simulated_soc: data[2],
            high_temp: data[3],
            low_temp: data[4],
        }
    }
}

trait CanReading {
    fn id() -> Id;
    fn construct(data: &[u8]) -> Self;
}

async fn read_can(){
    loop {
        let Ok(mut sock) = CanSocket::open(CAN_INTERFACE) else {
            eprintln!("Failed to open CAN socket, retrying...");
            tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;
            continue;
        };
        while let Some(Ok(frame)) = sock.next().await {
            let data = frame.data();
            let id = frame.id();


            if PackReading1::id() == id {
                if data.len() != PackReading1::SIZE {
                    eprintln!("Invalid data length for PackReading1: {}", data.len());
                    continue;
                }

                let reading = PackReading1::construct(data);

                #[cfg(debug_assertions)]
                println!("{:?}", reading);
            }

            if PackReading2::id() == id {
                if data.len() != PackReading2::SIZE {
                    eprintln!("Invalid data length for PackReading2: {}", data.len());
                    continue;
                }

                let reading = PackReading2::construct(data);

                #[cfg(debug_assertions)]
                println!("{:?}", reading);
            }
        }
    }
}