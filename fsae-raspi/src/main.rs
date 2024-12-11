mod influx;
mod can;
mod uart;
mod mqtt;
use tokio;
use can::read_can;
use uart::read_uart;
use mqtt::mqttd;


#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    tokio::spawn(mqttd());

    tokio::spawn(read_can());

    tokio::spawn(read_uart());
    
    tokio::signal::ctrl_c().await?;

    Ok(())
}
