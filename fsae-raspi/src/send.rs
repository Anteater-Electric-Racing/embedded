use reqwest::Client;
use rumqttc::{AsyncClient, MqttOptions, QoS};
use serde::Serialize;
use tokio::time::Duration;
use tracing::error;

use crate::influxdb::to_line_protocol;
use tokio::sync::OnceCell;

pub const INFLUXDB_URL: &str = "http://127.0.0.1:8181";
pub const INFLUXDB_DATABASE: &str = "fsae";

pub const MQTT_ID: &str = "fsae";
pub const MQTT_HOST: &str = "127.0.0.1";
pub const MQTT_PORT: u16 = 1883;

pub trait Reading: Serialize {
    fn topic() -> &'static str;
}

static INFLUX_CLIENT: OnceCell<Client> = OnceCell::const_new();
static MQTT_CLIENT: OnceCell<AsyncClient> = OnceCell::const_new();

async fn get_influx_client() -> &'static Client {
    INFLUX_CLIENT
        .get_or_init(|| async {
            reqwest::Client::builder()
                .build()
                .expect("Failed to build InfluxDB client")
        })
        .await
}

async fn get_mqtt_client() -> &'static AsyncClient {
    MQTT_CLIENT
        .get_or_init(|| async {
            let mut mqttoptions = MqttOptions::new(MQTT_ID, MQTT_HOST, MQTT_PORT);
            mqttoptions.set_keep_alive(Duration::from_secs(5));
            let (mqtt_client, mut eventloop) = AsyncClient::new(mqttoptions, 10);

            tokio::spawn(async move {
                loop {
                    if let Err(e) = eventloop.poll().await {
                        error!(%e, "MQTT eventloop error");
                        tokio::time::sleep(Duration::from_secs(1)).await;
                    }
                }
            });

            mqtt_client
        })
        .await
}

pub async fn send_message<T: Reading>(message: T) {
    let json = match serde_json::to_string(&message) {
        Ok(j) => j,
        Err(e) => {
            error!(%e, "Failed to serialize message");
            return;
        }
    };

    let line_protocol = to_line_protocol(&message);

    let mqtt_fut = async {
        if let Err(e) = get_mqtt_client()
            .await
            .publish(T::topic(), QoS::AtLeastOnce, false, json)
            .await
        {
            error!(%e, "Failed to publish to MQTT");
        }
    };

    let influx_fut = async {
        let url = format!(
            "{}/api/v3/write_lp?db={}&precision=nanosecond",
            INFLUXDB_URL, INFLUXDB_DATABASE
        );
        if let Err(e) = get_influx_client()
            .await
            .post(&url)
            .header("Content-Type", "text/plain")
            .body(line_protocol)
            .send()
            .await
        {
            error!(%e, "Failed to write to InfluxDB");
        }
    };

    tokio::spawn(mqtt_fut);
    tokio::spawn(influx_fut);
}
