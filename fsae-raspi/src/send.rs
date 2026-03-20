use std::sync::Arc;

use rumqttc::{AsyncClient, ClientError, MqttOptions, QoS};
use serde::Serialize;
use taos::taos_query::common::{SchemalessPrecision, SchemalessProtocol, SmlDataBuilder};
use taos::{AsyncQueryable, AsyncTBuilder, TaosBuilder};
use tokio::sync::mpsc::error::TrySendError;
use tokio::sync::mpsc::Sender;
use tokio::sync::{Mutex, OnceCell};
use tokio::time::Duration;
use tracing::error;

pub const TAOS_URL: &str = "taos+ws://localhost:6041/fsae";
pub const MQTT_ID: &str = "fsae";
pub const MQTT_HOST: &str = "127.0.0.1";
pub const MQTT_PORT: u16 = 1883;

const CREATE_DB: &str =
    "CREATE DATABASE IF NOT EXISTS fsae WAL_LEVEL 2 WAL_FSYNC_PERIOD 0 STT_TRIGGER 1 KEEP 365d";
const MAX_CONSECUTIVE_FAILURES: u32 = 10;

pub trait Reading: Serialize {
    fn topic() -> &'static str;
}

static TDENGINE: OnceCell<Sender<String>> = OnceCell::const_new();
static MQTT_CLIENT: OnceCell<AsyncClient> = OnceCell::const_new();

async fn get_tdengine_sender() -> &'static Sender<String> {
    TDENGINE
        .get_or_init(|| async {
            let (tx, rx) = tokio::sync::mpsc::channel(100_000);
            let rx = Arc::new(Mutex::new(rx));

            let builder =
                TaosBuilder::from_dsn(TAOS_URL).unwrap_or_else(|e| panic!("Invalid DSN: {e}"));

            {
                let taos = builder
                    .build()
                    .await
                    .unwrap_or_else(|e| panic!("Failed to connect to TDengine: {e}"));
                if let Err(e) = taos.exec(CREATE_DB).await {
                    error!(%e, "Failed to create database");
                }
            }

            for _ in 0..4 {
                let rx = rx.clone();
                let taos = builder
                    .build()
                    .await
                    .unwrap_or_else(|e| panic!("Failed to connect to TDengine: {e}"));
                tokio::spawn(async move {
                    let mut buffer: Vec<String> = Vec::new();
                    let mut id: u64 = 0;
                    let mut consecutive_failures: u32 = 0;

                    while rx.lock().await.recv_many(&mut buffer, 5000).await > 0 {
                        let data = SmlDataBuilder::default()
                            .protocol(SchemalessProtocol::Line)
                            .precision(SchemalessPrecision::Millisecond)
                            .data(std::mem::take(&mut buffer))
                            .req_id(id)
                            .build()
                            .unwrap_or_else(|e| panic!("Failed to build SML data: {e}"));
                        id = id.wrapping_add(1);

                        match taos.put(&data).await {
                            Ok(_) => {
                                consecutive_failures = 0;
                            }
                            Err(e) => {
                                consecutive_failures += 1;
                                error!(%e, consecutive_failures, "Failed to insert into TDengine");

                                if consecutive_failures >= MAX_CONSECUTIVE_FAILURES {
                                    error!(
                                        "Hit {MAX_CONSECUTIVE_FAILURES} consecutive write failures — recreating database"
                                    );
                                    let _ = taos.exec("DROP DATABASE IF EXISTS fsae").await;
                                    let _ = taos.exec(CREATE_DB).await;
                                    consecutive_failures = 0;
                                }
                            }
                        }
                    }
                });
            }

            tx
        })
        .await
}

async fn get_mqtt_client() -> &'static AsyncClient {
    MQTT_CLIENT
        .get_or_init(|| async {
            let opts = MqttOptions::new(MQTT_ID, MQTT_HOST, MQTT_PORT);
            let (client, mut eventloop) = AsyncClient::new(opts, 100_000);
            tokio::spawn(async move {
                loop {
                    if let Err(e) = eventloop.poll().await {
                        error!(%e, "MQTT eventloop error");
                        tokio::time::sleep(Duration::from_secs(1)).await;
                    }
                }
            });
            client
        })
        .await
}

#[inline]
fn push_field(buf: &mut String, k: &str, v: &serde_json::Value) {
    buf.push_str(k);
    buf.push('=');
    match v {
        serde_json::Value::Bool(b) => {
            buf.push_str(if *b { "true" } else { "false" });
        }
        serde_json::Value::Number(n) => {
            if let Some(f) = n.as_f64() {
                buf.push_str(ryu::Buffer::new().format(f));
                buf.push_str("f32");
            } else if let Some(i) = n.as_i64() {
                buf.push_str(itoa::Buffer::new().format(i));
                buf.push_str("i32");
            }
        }
        other => {
            buf.push('"');
            buf.push_str(&other.to_string());
            buf.push('"');
        }
    }
}

fn to_line_protocol_from_value(
    measurement: &str,
    map: &serde_json::Value,
    timestamp_ms: u64,
) -> Option<String> {
    let obj = map.as_object()?;
    let mut buf = String::with_capacity(measurement.len() + 1 + obj.len() * 30 + 20);
    buf.push_str(measurement);
    buf.push(' ');

    let mut iter = obj.iter();
    if let Some((k, v)) = iter.next() {
        push_field(&mut buf, k, v);
    }
    for (k, v) in iter {
        buf.push(',');
        push_field(&mut buf, k, v);
    }

    buf.push(' ');
    buf.push_str(itoa::Buffer::new().format(timestamp_ms));

    Some(buf)
}

pub fn now_ms() -> u64 {
    std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .expect("clock went backwards")
        .as_millis() as u64
}

pub async fn send_message<T: Reading + Send + 'static>(message: T, timestamp_ms: u64) {
    let mut value = match serde_json::to_value(&message) {
        Ok(v) => v,
        Err(e) => {
            error!(%e, "Failed to serialize message");
            return;
        }
    };

    if let Some(obj) = value.as_object_mut() {
        obj.insert("ts".to_string(), serde_json::json!(timestamp_ms));
    }

    let json = value.to_string();
    let topic = T::topic();

    match get_mqtt_client()
        .await
        .try_publish(topic, QoS::AtMostOnce, false, json)
    {
        Ok(()) => {}
        Err(ClientError::TryRequest(_)) => {
            tracing::warn!("MQTT channel full — dropping message");
        }
        Err(e) => error!(%e, "MQTT publish error"),
    }
    match get_tdengine_sender().await.try_send(
        match to_line_protocol_from_value(topic, &value, timestamp_ms) {
            Some(line) => line,
            None => {
                error!("Failed to convert to line protocol");
                return;
            }
        },
    ) {
        Ok(()) => {}
        Err(TrySendError::Full(_)) => {
            tracing::warn!("TDengine channel full — dropping message");
        }
        Err(e) => error!(%e, "Failed to send to TDengine channel"),
    }
}
