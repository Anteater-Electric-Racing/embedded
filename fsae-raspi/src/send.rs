use rumqttc::{AsyncClient, MqttOptions, QoS};
use serde::Serialize;
use std::cell::RefCell;
use std::fmt::Write;
use taos::taos_query::common::{SchemalessPrecision, SchemalessProtocol, SmlDataBuilder};
use taos::{AsyncQueryable, AsyncTBuilder, Taos, TaosBuilder};
use tokio::sync::OnceCell;
use tokio::time::Duration;
use tracing::{error, info};

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

static TAOS: OnceCell<Taos> = OnceCell::const_new();
static MQTT_CLIENT: OnceCell<AsyncClient> = OnceCell::const_new();

async fn get_taos_client() -> &'static Taos {
    TAOS.get_or_init(|| async {
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
        taos
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

fn to_line_protocol_from_value(measurement: &str, map: &serde_json::Value) -> Option<String> {
    let obj = map.as_object()?;
    let mut buf = String::with_capacity(552);
    buf.push_str(measurement);
    buf.push(' ');

    let mut first = true;
    for (k, v) in obj {
        if !first {
            buf.push(',');
        }
        first = false;
        match v {
            serde_json::Value::Bool(b) => write!(buf, "{k}={b}").unwrap(),
            serde_json::Value::Number(n) => {
                if n.is_f64() {
                    write!(buf, "{k}={n}f32").unwrap();
                } else {
                    write!(buf, "{k}={n}i32").unwrap();
                }
            }
            other => write!(buf, "{k}=\"{other}\"").unwrap(),
        }
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
                .publish(topic, QoS::AtLeastOnce, false, json)
                .await
            {
                error!(%e, "Failed to publish to MQTT");
            }
        },
        async {
            // let data = SmlDataBuilder::default()
            //     .protocol(SchemalessProtocol::Line)
            //     .precision(SchemalessPrecision::Millisecond)
            //     .data(vec![line])
            //     .ttl(1000)
            //     .req_id(100u64)
            //     .build()
            //     .unwrap();
            // if let Err(e) = get_taos_client().await.put(&data).await {
            //     error!(%e, "Failed to insert into TDengine");
            // }
        }
    );
}
