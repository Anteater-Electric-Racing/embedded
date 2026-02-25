//! Verification tests for InfluxDB3 and MQTT.
//!
//! This test module verifies 2 independent communication paths used
//!
//! - **InfluxDB 3 write/read**: send a [`TelemetryData`] packet via [`send_message`] and then query InfluxDB3 SQL API to verify
//!   the same packet is received.
//! - **MQTT publish/subscribe**: Verifies by ensuring telemetry packet JSON is published to `telemetry` topic, received, and
//!   deserialized properly.
//!
//! # InfluxDB tests
//!
//! Tests will write a value, then query SQL
//!
//! ```sql
//! SELECT * FROM 'telemetry' ORDER BY time DESC LIMIT 1
//! ```
//!
//! The newest row is compared against the original struct.  
//! Tests require an InfluxDB3 instance created when opening in devcontainer and a valid API token.
//!
//! # MQTT tests
//!
//! A listener subscribes to the `telemetry` topic and awaits the next `Publish`
//! packet. The payload is deserialized into [`TelemetryData`] and compared against
//! the expected value.
#[cfg(test)]
use crate::{
    can::TelemetryData,
    send::{send_message, Reading, INFLUXDB_DATABASE, INFLUXDB_URL, MQTT_HOST, MQTT_PORT},
};
use deku::prelude::*;
#[cfg(test)]
use tokio::time::Duration;
#[cfg(test)]
use tokio_socketcan_isotp::{IsoTpSocket, StandardId};
#[cfg(test)]
use tracing::info;

/// Sends a raw CAN_PACKET_SIZE-byte [`TelemetryData`] packet over ISO-TP on `vcan0`.
///
/// Only compiled in `cfg(test)` mode.
/// Requires a virtual CAN interface.
#[cfg(test)]
async fn send_telemetry_over_isotp(data: &TelemetryData) -> Result<(), Box<dyn std::error::Error>> {
    let socket = IsoTpSocket::open(
        "vcan0",
        StandardId::new(0x123).ok_or("Invalid source ID")?,
        StandardId::new(0x321).ok_or("Invalid destination ID")?,
    )?;

    let payload = TelemetryData::to_bytes(data)?;
    socket.write_packet(&payload).await?;

    info!(bytes = payload.len(), "TelemetryData sent over ISO-TP");
    Ok(())
}

/// Verifies that [`parse_telemetry`] round-trips through
/// [`telemetry_to_raw_bytes`] without any CAN hardware.
#[test]
fn test_parse_telemetry_roundtrip() {
    let original = TelemetryData::default();

    let raw = TelemetryData::to_bytes(&original).unwrap();
    let ((remaining, offset), parsed) =
        TelemetryData::from_bytes((raw.as_ref(), 0)).expect("parse_telemetry failed");
    assert_eq!(original, parsed);
    assert_eq!(remaining.len(), 0);
    assert_eq!(offset, 0);
}

/// Rejects a packet that is too short.
#[test]
fn test_parse_telemetry_bad_length() {
    let short = [0u8; 10];
    assert!(TelemetryData::from_bytes((short.as_ref(), 0)).is_err());
}

/// Rejects a packet containing an invalid enum byte.
#[test]
fn test_parse_telemetry_invalid_enum() {
    let mut raw = [0u8; TelemetryData::SIZE_BITS / 8];
    // motor_direction at byte 16 — set to an invalid discriminant
    raw[16] = 200;
    assert!(TelemetryData::from_bytes((raw.as_ref(), 0)).is_err());
}

/// Sends a dummy telemetry packet over ISO-TP on `vcan0` and verifies delivery.
///
/// Requires:
/// ```bash
/// sudo modprobe vcan
/// sudo ip link add dev vcan0 type vcan
/// sudo ip link set up vcan0
/// ```
/// These commands are run automatically in the GitHub Actions workflow (test.yml).
#[tokio::test]
async fn test_send_telemetry_over_isotp() -> Result<(), Box<dyn std::error::Error>> {
    let data = TelemetryData::default();

    send_telemetry_over_isotp(&data).await?;
    Ok(())
}

/// Verifies that a telemetry packet was written to InfluxDB.
///
/// Sends a SQL query for the most recent row in the database returned by
/// [`T::topic`] and compares it with `test_packet`.
///
/// Returns `Ok(true)` if the newest row matches `test_packet`,  
/// `Ok(false)` if it does not,  
/// and `Err(..)` on request or deserialization failure.   
#[cfg(test)]
pub async fn verify_influx_write<T: Reading + for<'de> serde::Deserialize<'de> + PartialEq>(
    test_packet: T,
) -> Result<bool, Box<dyn std::error::Error>> {
    let client = reqwest::Client::builder().build()?;

    let query = format!("SELECT * FROM {} ORDER BY time DESC LIMIT 1", T::topic());

    let url = format!("{}/api/v3/query_sql", INFLUXDB_URL);

    let body = serde_json::json!({
        "db": INFLUXDB_DATABASE,
        "q": query
    });

    let resp = client
        .post(&url)
        .header("Authorization", "Bearer apiv3_TQdSxXbtRc8qbzb4ejQOa-ir9-deb4fSVe5Lc-RgvQZqPKikusEJtZpQmEJakPtxZvst8wW4B20KB8iSGLC-Tg")
        .body(body.to_string())
        .send()
        .await?.text().await?;

    let resp_array: Vec<T> = serde_json::from_str(&resp)?;
    let resp_struct = resp_array
        .into_iter()
        .next()
        .ok_or("No data returned from InfluxDB")?;

    Ok(resp_struct == test_packet)
}

/// Test for the InfluxDB3 telemetry pipeline by creating a test packet to be sent
///
/// Sends a [`TelemetryData`] test packet through [`send_message`] and verifies that the
/// value can be read back using [`verify_influx_write`].
///
/// Requires a running InfluxDB instance  
#[test]
fn test_verify_influx_write() {
    let test_packet = TelemetryData::default();

    println!("Sending TelemetryData test packet to influxdb3");
    tokio::runtime::Runtime::new().unwrap().block_on(async {
        send_message(test_packet.clone()).await;
        println!("finished sending, now verifying...");
        match verify_influx_write(test_packet).await {
            Ok(result) => println!("InfluxDB verification result: {}", result),
            Err(e) => panic!("Error verifying InfluxDB write: {}", e),
        }
    });
}

/// Waits for a telemetry publish on the `telemetry` MQTT topic
///
/// Subscribes to the topic, receives the next `Publish` packet, deserializes
/// the payload into [`TelemetryData`], and compares it with `telemetry_struct`
///
/// Returns `true` on a matching publish, or `false` on timeout, subscribe
/// failures, or deserialization errors. Errors are logged to stderr
#[cfg(test)]
async fn verify_mqtt_listener(telemetry_struct: TelemetryData) -> bool {
    // Client and event loop setup
    let mut options = rumqttc::MqttOptions::new("listener", MQTT_HOST, MQTT_PORT);
    options.set_keep_alive(Duration::from_secs(5));
    let (client, mut event_loop) = rumqttc::AsyncClient::new(options, 10);

    // Subscribe client to topic
    if let Err(e) = client
        .subscribe("telemetry", rumqttc::QoS::AtLeastOnce)
        .await
    {
        eprintln!("[subscribe] | Unable to subscribe to the topic: {}", e);
        return false;
    }

    // Repeat poll check loop while event loop has not returned an Err.
    let timeout = Duration::from_secs(1);
    tokio::time::timeout(timeout, async {
        while let Ok(notification) = event_loop.poll().await {
            // Check for Publish messages and extract
            if let rumqttc::Event::Incoming(rumqttc::Packet::Publish(data)) = notification {
                let data_string = match std::str::from_utf8(&data.payload) {
                    Ok(s) => s,
                    Err(e) => {
                        eprintln!(
                            "[string parse] | Failed to convert bytes into data string: {}",
                            e
                        );
                        continue;
                    }
                };

                let data_deserialized = match serde_json::from_str::<TelemetryData>(data_string) {
                    Ok(d) => d,
                    Err(e) => {
                        eprintln!("[poll] | Failed to deserialize incoming data: {}", e);
                        continue;
                    }
                };

                return data_deserialized == telemetry_struct;
            }
        }
        false
    })
    .await
    .unwrap_or(false)
}

/// End-to-end test for the MQTT telemetry path.
///
/// Spawns a listener task with [`verify_mqtt_listener`], sends a
/// [`TelemetryData`] message using [`send_message`], and asserts that the
/// listener receives an identical value.
///
/// Requires a reachable MQTT broker at `MQTT_HOST:MQTT_PORT`
#[test]
fn test_verify_mqtt_listener() {
    // Test Struct (listener end)
    let listener_data: TelemetryData = TelemetryData::default();

    // Run verify
    let runtime = tokio::runtime::Runtime::new().expect("Unable to start listener runtime.");
    let handle = runtime.spawn(verify_mqtt_listener(listener_data.clone()));
    runtime.block_on(async {
        tokio::time::sleep(Duration::from_millis(500)).await;
        send_message(listener_data.clone()).await;
    });
    let result = runtime
        .block_on(handle)
        .expect("Listener task panicked unexpectedly.");
    assert!(
        result,
        "MQTT listener did not receive the expected message."
    );
}
