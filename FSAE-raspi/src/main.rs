mod messages;
use tokio;
use messages::read_can;
use influxdb::{Client, Error, InfluxDbWriteable, ReadQuery, Timestamp};


#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    tokio::spawn(read_can());
    
    tokio::signal::ctrl_c().await?;

    Ok(())
}
