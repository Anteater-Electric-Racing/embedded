mod influx;
mod can;
mod uart;
use tokio;
use can::read_can;
use uart::read_uart;


#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    tokio::spawn(read_can());

    tokio::spawn(read_uart());
    
    tokio::signal::ctrl_c().await?;

    Ok(())
}
