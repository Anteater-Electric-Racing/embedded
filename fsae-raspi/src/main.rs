mod can;
mod influxdb;
mod mqtt;
mod send;
use can::read_can;
use mqtt::mqttd;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    tokio::spawn(read_can());

    mqttd();

    Ok(())
}
