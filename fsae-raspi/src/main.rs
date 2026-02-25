mod can;
mod influxdb;
mod mqtt;
mod send;
#[cfg(test)]
mod test;

use can::read_can;
use mqtt::mqttd;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialise tracing before spawning any tasks.
    tracing_subscriber::fmt()
        .pretty()
        .with_line_number(false)
        .with_file(false)
        .with_thread_ids(false)
        .with_thread_names(false)
        .with_env_filter("rumqttd=warn")
        .try_init()
        .expect("initialised tracing subscriber");

    tokio::spawn(read_can());

    mqttd();

    Ok(())
}
