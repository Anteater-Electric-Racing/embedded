use rumqttc::{AsyncClient, MqttOptions, QoS};
use serde::Serialize;
use std::sync::Arc;
use taos::taos_query::common::{SchemalessPrecision, SchemalessProtocol, SmlDataBuilder};
use taos::{AsyncQueryable, AsyncTBuilder, TaosBuilder};
use tokio::sync::mpsc::error::TrySendError;
use tokio::sync::mpsc::Sender;
use tokio::sync::Mutex;
use tokio::sync::OnceCell;
use tokio::time::Duration;
use tracing::error;

pub const TAOS_URL: &str = "taos://localhost:6030";
pub const TAOS_DATABASE: &str = "fsae";
pub const MQTT_ID: &str = "fsae";
pub const MQTT_HOST: &str = "127.0.0.1";
pub const MQTT_PORT: u16 = 1883;

pub trait Reading: Serialize {
    fn topic() -> &'static str;
    fn measurement() -> &'static str;
    // fn to_line_protocol(&self) -> String;
}

static TDENGINE: OnceCell<Sender<String>> = OnceCell::const_new();
static MQTT_CLIENT: OnceCell<AsyncClient> = OnceCell::const_new();

async fn get_tdengine_sender() -> &'static Sender<String> {
    TDENGINE
        .get_or_init(|| async {
            let (tx, mut rx) = tokio::sync::mpsc::channel::<String>(100_000);

            let builder =
                TaosBuilder::from_dsn(TAOS_URL).unwrap_or_else(|e| panic!("Invalid DSN: {e}"));
            let taos = builder
                .build()
                .await
                .unwrap_or_else(|e| panic!("Failed to connect to TDengine: {e}"));
            if let Err(e) = taos
                .exec(format!("CREATE DATABASE IF NOT EXISTS {TAOS_DATABASE}"))
                .await
            {
                error!(%e, "Failed to create database");
            }
            if let Err(e) = taos.exec(format!("USE {TAOS_DATABASE}")).await {
                error!(%e, "Failed to use database");
            }

            tokio::spawn(async move {
                let mut buffer: Vec<String> = Vec::new();
                let mut id: u64 = 0;
                while rx.recv_many(&mut buffer, 10_000).await > 0 {
                    let batch_size = buffer.len();
                    if batch_size > 5000 {
                        tracing::warn!(
                            batch_size,
                            "Large TDEngine batch — ingest channel may be overloaded"
                        );
                    }

                    let data = SmlDataBuilder::default()
                        .protocol(SchemalessProtocol::Line)
                        .precision(SchemalessPrecision::Millisecond)
                        .data(std::mem::take(&mut buffer))
                        .req_id(id)
                        .build()
                        .unwrap();
                    id += 1;
                    if let Err(e) = taos.put(&data).await {
                        error!(%e, "Failed to insert into TDengine");
                    }
                }
            });

            tx
        })
        .await
}

async fn get_mqtt_client() -> &'static AsyncClient {
    MQTT_CLIENT
        .get_or_init(|| async {
            let mut opts = MqttOptions::new(MQTT_ID, MQTT_HOST, MQTT_PORT);
            opts.set_keep_alive(Duration::from_secs(5));
            let (client, mut eventloop) = AsyncClient::new(opts, 10);
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
                buf.push_str(zmij::Buffer::new().format(f));
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

fn to_line_protocol_from_value(measurement: &str, map: &serde_json::Value) -> Option<String> {
    let obj = map.as_object()?;
    let mut buf = String::with_capacity(measurement.len() + 1 + obj.len() * 30);
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

    Some(buf)
}

pub async fn send_message<T: Reading + Send + 'static>(message: T) {
    let value = match serde_json::to_value(&message) {
        Ok(v) => v,
        Err(e) => {
            error!(%e, "Failed to serialize message");
            return;
        }
    };

    let json = value.to_string();
    let topic = T::topic();
    let line = match to_line_protocol_from_value(T::measurement(), &value) {
        Some(l) => l,
        None => {
            error!("Failed to build line protocol");
            return;
        }
    };

    tokio::join!(
        async {
            if let Err(e) = get_mqtt_client()
                .await
                .publish(topic, QoS::AtMostOnce, false, json)
                .await
            {
                error!(%e, "Failed to publish to MQTT — broker or eventloop may be overloaded");
            }
        },
        async {
            let sender = get_tdengine_sender().await;

            let remaining = sender.capacity();
            if remaining < 20 {
                tracing::warn!(
                    remaining_capacity = remaining,
                    "TDEngine ingest channel is nearly full — writer may be falling behind"
                );
            }

            match sender.try_send(line) {
                Ok(()) => {}
                Err(TrySendError::Full(msg)) => {
                    tracing::warn!(
                        "TDEngine ingest channel is full — dropping line protocol message: {msg}"
                    );
                }
                Err(TrySendError::Closed(_)) => {
                    error!("TDEngine ingest channel is closed — writer task has exited");
                }
            }
        }
    );
}
