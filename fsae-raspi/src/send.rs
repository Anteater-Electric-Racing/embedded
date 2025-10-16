use rumqttc::{AsyncClient, MqttOptions, QoS};
use serde::Serialize;
use std::sync::LazyLock;
use tokio::time::Duration;
use influxdb3_client::{Client, Precision};
use serde_json::Value as JsonValue;
use chrono::{DateTime, Utc};
use std::path::PathBuf;
use std::env;

pub const INFLUXDB_URL: &str = "http://127.0.0.1:8181";
pub const INFLUXDB_DATABASE: &str = "fsae";
// pub const INFLUXDB_CA_CERT: &str = None; expecting a string not object

pub const MQTT_ID: &str = "fsae";
pub const MQTT_HOST: &str = "127.0.0.1";
pub const MQTT_PORT: u16 = 1883;

pub trait Reading: Serialize {
    fn topic() -> &'static str;
}

static INFLUX_CLIENT: LazyLock<Client> = LazyLock::new(|| {
   
    let ca_cert = env::var("INFLUXDB_CA_CERT").ok().map(PathBuf::from);

    Client::new(INFLUXDB_URL, ca_cert).expect("cant create client:")
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

pub async fn send_message<T: Reading + Send + 'static>(message: T) {
    let json = match serde_json::to_string(&message) {
        Ok(j) => j,
        Err(e) => {
            eprintln!("Failed to serialize: {}", e);
            return;
        }
    };

    println!("Sending message: {}", json);

    if let Err(e) = MQTT_CLIENT
        .publish(T::topic(), QoS::AtLeastOnce, false, json)
        .await
    {
        eprintln!("Failed to publish to MQTT: {}", e);
    }

    let lp = match to_line_protocol(T::topic(), &message) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Failed to build line-protocol: {}", e);
            return;
        }
    };

    println!("Line-protocol: {}", lp);

    let write_lp = INFLUX_CLIENT
        .api_v3_write_lp(INFLUXDB_DATABASE)
        .precision(Precision::Nanosecond)
        .accept_partial(true)
        .body(lp);

    if let Err(e) = write_lp.send().await {
        eprintln!("Failed to write to InfluxDB3: {}", e);
    }
}

fn escape_key(k: &str) -> String {
    k.replace(' ', "\\ ").replace(',', "\\,").replace('=', "\\=")
}

fn escape_string_field(v: &str) -> String {
    format!("\"{}\"", v.replace('"', "\\\""))
}

fn to_line_protocol<T: Serialize>(measurement: &str, value: &T) -> Result<String, String> {
    let json = serde_json::to_value(value).map_err(|e| e.to_string())?;

    let mut fields = Vec::new();
    let mut timestamp_ns: Option<i128> = None;

    if let JsonValue::Object(map) = json {
        for (k, v) in map {
            if k == "time" {
                           if let JsonValue::String(ref s) = v {
                    if let Ok(dt) = s.parse::<DateTime<Utc>>() {
                        let ts_ns = (dt.timestamp() as i128) * 1_000_000_000
                            + (dt.timestamp_subsec_nanos() as i128);
                        timestamp_ns = Some(ts_ns);
                        continue;
                    }
                }
            }
            let key = escape_key(&k);
            let field_value = match &v {
                JsonValue::String(s) => escape_string_field(&s),
                JsonValue::Number(n) => {
                    if n.is_i64() {
                        format!("{}i", n.as_i64().unwrap())
                    } else {
                        format!("{}", n.as_f64().unwrap())
                    }
                }
                JsonValue::Bool(b) => b.to_string(),
                _ => escape_string_field(&v.to_string()),
            };

            fields.push(format!("{}={}", key, field_value));
        }
    } else {
           fields.push(format!("value={}", escape_string_field(&json.to_string())));
    }

    let fields_part = fields.join(",");

    let ts = timestamp_ns.unwrap_or_else(|| {
    let now = Utc::now();
    (now.timestamp() as i128) * 1_000_000_000
        + (now.timestamp_subsec_nanos() as i128)
    });
    Ok(format!("{},{} {}", measurement, fields_part, ts))
}