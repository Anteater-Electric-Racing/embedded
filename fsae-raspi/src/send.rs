use reqwest::Client;
use rumqttc::{AsyncClient, MqttOptions, QoS};
use serde::Serialize;
use std::sync::LazyLock;
use tokio::time::Duration;

use crate::influxdb::to_line_protocol;

pub const INFLUXDB_URL: &str = "http://localhost:8181";

pub const INFLUXDB_DATABASE: &str = "fsae";

pub const MQTT_ID: &str = "fsae";
pub const MQTT_HOST: &str = "127.0.0.1";
pub const MQTT_PORT: u16 = 1883;

pub trait Reading: Serialize {
    fn topic() -> &'static str;
}

pub static INFLUX_CLIENT: LazyLock<Client> = LazyLock::new(|| {
    reqwest::Client::builder()
        .build()
        .expect("Failed to build InfluxDB client")
});

static MQTT_CLIENT: LazyLock<AsyncClient> = LazyLock::new(|| {
    let mut mqttoptions = MqttOptions::new(MQTT_ID, MQTT_HOST, MQTT_PORT);
    mqttoptions.set_keep_alive(Duration::from_secs(5));
    let (mqtt_client, mut eventloop) = AsyncClient::new(mqttoptions, 10);

    tokio::spawn(async move {
        loop {
            if let Err(e) = eventloop.poll().await {
                eprintln!("MQTT eventloop error: {}", e);
                tokio::time::sleep(Duration::from_secs(1)).await;
            }
        }
    });

    mqtt_client
});

pub async fn send_message<T: Reading>(message: T) {
    let json = match serde_json::to_string(&message) {
        Ok(j) => j,
        Err(e) => {
            eprintln!("Failed to serialize: {}", e);
            return;
        }
    };

    if let Err(e) = MQTT_CLIENT
        .publish(T::topic(), QoS::AtLeastOnce, false, json)
        .await
    {
        eprintln!("Failed to publish to MQTT: {}", e);
    }

    let line_protocol = to_line_protocol(&message);
    let url = format!(
        "{}/api/v3/write_lp?db={}&precision=nanosecond&accept_partial=true&no_sync=false",
        INFLUXDB_URL, INFLUXDB_DATABASE
    );
    if let Err(e) = INFLUX_CLIENT
        .post(&url)
        .header("Authorization", "Bearer apiv3_TQdSxXbtRc8qbzb4ejQOa-ir9-deb4fSVe5Lc-RgvQZqPKikusEJtZpQmEJakPtxZvst8wW4B20KB8iSGLC-Tg")
        .body(line_protocol)
        .send()
        .await
    {
        eprintln!("Failed to write to InfluxDB: {}", e);
    }
}
