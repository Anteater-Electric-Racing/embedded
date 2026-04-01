//! Verification tests for TDengine and MQTT.
//!
//! This test module verifies 2 independent communication paths used
//!
//! - **TDengine write/read**: send a [`TelemetryData`] packet via [`send_message`] and then query TDengine to verify
//!   the same packet is received.
//! - **MQTT publish/subscribe**: Verifies by ensuring telemetry packet JSON is published to `telemetry` topic, received, and
//!   deserialized properly.
//!
//! # TDengine tests
//!
//! Tests will write a value, then query SQL
//!
//! ```sql
//! SELECT * FROM telemetry ORDER BY _ts DESC LIMIT 1
//! ```
//!
//! The newest row is compared against the original struct.  
//! Tests require a TDengine instance created when opening in devcontainer.
//!
//! # MQTT tests
//!
//! A listener subscribes to the `telemetry` topic and awaits the next `Publish`
//! packet. The payload is deserialized into [`TelemetryData`] and compared against
//! the expected value.
#[cfg(test)]
use crate::{
    can::TelemetryData,
    mqtt::mqttd,
    send::{now_ms, send_message, Reading, MQTT_HOST, MQTT_PORT},
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
    // motor_direction at byte 56 — set to an invalid discriminant
    raw[56] = 200;
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
#[ignore]
async fn test_send_telemetry_over_isotp() -> Result<(), Box<dyn std::error::Error>> {
    let data = TelemetryData::default();

    send_telemetry_over_isotp(&data).await?;
    Ok(())
}

/// Verifies that a telemetry packet was written to TDengine.
///
/// Sends a SQL query via the TDengine REST API for the most recent row in
/// the table returned by [`T::topic`] and compares it with `test_packet`.
///
/// Returns `Ok(true)` if the newest row matches `test_packet`,
/// `Ok(false)` if it does not,
/// and `Err(..)` on request or deserialization failure.
#[cfg(test)]
pub async fn verify_tdengine_write<
    T: Reading + serde::Serialize + for<'de> serde::Deserialize<'de> + PartialEq,
>(
    test_packet: T,
) -> Result<bool, Box<dyn std::error::Error>> {
    let client = reqwest::Client::builder().build()?;

    let query = format!("SELECT * FROM {} ORDER BY _ts DESC LIMIT 1", T::topic());

    // TDengine REST API on the same host/port as the WebSocket DSN
    let resp = client
        .post("http://localhost:6041/rest/sql/fsae")
        .header("Authorization", "Basic cm9vdDp0YW9zZGF0YQ==") // root:taosdata
        .body(query)
        .send()
        .await?
        .text()
        .await?;

    let resp_json: serde_json::Value = serde_json::from_str(&resp)?;

    let column_meta = resp_json["column_meta"]
        .as_array()
        .ok_or("Missing column_meta")?;
    let rows = resp_json["data"].as_array().ok_or("Missing data")?;

    let row = rows.first().ok_or("No data returned from TDengine")?;
    let row = row.as_array().ok_or("Row is not an array")?;

    let expected = serde_json::to_value(&test_packet)?;
    let expected_obj = expected.as_object().ok_or("Expected object")?;

    for (i, meta) in column_meta.iter().enumerate() {
        let col_name = meta[0].as_str().unwrap_or_default();
        if col_name == "_ts" || col_name == "ts" {
            continue;
        }
        if let Some(expected_val) = expected_obj.get(col_name) {
            let row_val = &row[i];
            match (expected_val, row_val) {
                (serde_json::Value::Number(a), serde_json::Value::Number(b)) => {
                    let a = a.as_f64().unwrap_or_default();
                    let b = b.as_f64().unwrap_or_default();
                    if (a - b).abs() > 1e-6 {
                        return Ok(false);
                    }
                }
                (serde_json::Value::Bool(a), serde_json::Value::Bool(b)) => {
                    if a != b {
                        return Ok(false);
                    }
                }
                _ => {
                    if *expected_val != *row_val {
                        return Ok(false);
                    }
                }
            }
        }
    }

    Ok(true)
}

/// Test for the TDengine telemetry pipeline by creating a test packet to be sent
///
/// Sends a [`TelemetryData`] test packet through [`send_message`] and verifies that the
/// value can be read back using [`verify_tdengine_write`].
///
/// Requires a running TDengine instance
#[test]
fn test_verify_tdengine_write() {
    let test_packet = TelemetryData::default();

    println!("Sending TelemetryData test packet to TDengine");
    tokio::runtime::Runtime::new().unwrap().block_on(async {
        send_message(test_packet.clone(), now_ms()).await;
        tokio::time::sleep(Duration::from_millis(500)).await;
        println!("finished sending, now verifying...");
        match verify_tdengine_write(test_packet).await {
            Ok(result) => println!("TDengine verification result: {}", result),
            Err(e) => panic!("Error verifying TDengine write: {}", e),
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
    let timeout = Duration::from_secs(5);
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

                let mut map: serde_json::Map<String, serde_json::Value> =
                    match serde_json::from_str(data_string) {
                        Ok(m) => m,
                        Err(e) => {
                            eprintln!("[poll] | Failed to deserialize incoming data: {}", e);
                            continue;
                        }
                    };
                map.remove("ts");

                let data_deserialized: TelemetryData =
                    match serde_json::from_value(serde_json::Value::Object(map)) {
                        Ok(d) => d,
                        Err(e) => {
                            eprintln!("[poll] | Failed to convert map to TelemetryData: {}", e);
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
    std::thread::spawn(mqttd);
    std::thread::sleep(std::time::Duration::from_millis(500));

    // Test Struct (listener end)
    let listener_data: TelemetryData = TelemetryData::default();

    // Run verify
    let runtime = tokio::runtime::Runtime::new().expect("Unable to start listener runtime.");
    let handle = runtime.spawn(verify_mqtt_listener(listener_data.clone()));
    runtime.block_on(async {
        tokio::time::sleep(Duration::from_millis(500)).await;
        send_message(listener_data.clone(), now_ms()).await;
    });
    let result = runtime
        .block_on(handle)
        .expect("Listener task panicked unexpectedly.");
    assert!(
        result,
        "MQTT listener did not receive the expected message."
    );
}
