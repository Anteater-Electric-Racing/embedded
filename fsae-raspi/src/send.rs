use influxdb::{Client, InfluxDbWriteable};
use rumqttc::{AsyncClient, MqttOptions, QoS};
use serde::Serialize;
use tokio::time::Duration;

pub const INFLUXDB_URL: &str = "http://127.0.0.1:8086";
pub const INFLUXDB_DATABASE: &str = "fsae-raspi";

pub const MQTT_ID: &str = "fsae-raspi";
pub const MQTT_HOST: &str = "127.0.0.1";
pub const MQTT_PORT: u16 = 1883;

pub trait Reading: InfluxDbWriteable + Serialize {
    fn topic() -> &'static str;
}

#[derive(Clone, Debug)]
pub struct Sender {
    influx_client: Client,
    mqtt_client: AsyncClient,
}

impl Sender {
    pub fn new() -> Self {
        let influx_client = Client::new(INFLUXDB_URL, INFLUXDB_DATABASE);

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

        Self {
            influx_client,
            mqtt_client,
        }
    }

    pub async fn send_message<T: Reading>(
        &self,
        message: T,
    ) {
        let json = match serde_json::to_string(&message) {
            Ok(j) => j,
            Err(e) => {
                eprintln!("Failed to serialize: {}", e);
                return;
            }
        };
    
        if let Err(e) = self.mqtt_client
            .publish(T::topic(), QoS::AtLeastOnce, false, json)
            .await
        {
            eprintln!("Failed to publish to MQTT: {}", e);
        }
    
        if let Err(e) = self.influx_client.query(message.into_query(T::topic())).await {
            eprintln!("Failed to write to InfluxDB: {}", e);
        }
    }
}


