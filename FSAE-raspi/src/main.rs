use chrono::{format, DateTime, Utc};
use FSAE_macro::MessageSize;

#[derive(MessageSize)]
struct PackReading1 {
    time: DateTime<Utc>,
    current: i16,
    inst_voltage: i16,
}

fn main() {
    println!("{}", PackReading1::SIZE);
}
