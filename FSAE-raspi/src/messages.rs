use chrono::{DateTime, Utc};
use fsae_macro::MessageSize;
use socketcan::{tokio::CanSocket, Id, Result, StandardId};

trait CanReading {
    fn id(&self) -> Id;
    const NAME: &'static str;
}

#[derive(MessageSize)]
struct PackReading1 {
    time: DateTime<Utc>,
    current: i16,
    inst_voltage: i16,
}

impl CanReading for PackReading1 {
    fn id(&self) -> Id {
        Id::Standard(StandardId::new(0x03B).unwrap())
    }
    const NAME: &str = "pack1";
}

fn test(){

}