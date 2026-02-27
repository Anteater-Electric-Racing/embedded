mod can;
mod mqtt;
mod send;
#[cfg(test)]
mod test;

use can::read_can;
use mqtt::mqttd;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    tracing_subscriber::fmt()
        .pretty()
        .with_env_filter("info")
        .init();

    tokio::spawn(read_can());

    mqttd();

    Ok(())
}
