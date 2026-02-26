mod can;
mod mqtt;
mod send;
#[cfg(test)]
mod test;
use std::thread;

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
        .with_env_filter("info,rumqttd=warn")
        .try_init()
        .expect("initialised tracing subscriber");

    thread::spawn(mqttd);

    read_can().await;

    Ok(())
}
