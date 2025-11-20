//! Verification tests for InfluxDB3 and MQTT.
//!
//! This test module verifies 2 independent communication paths used
//!
//! - **InfluxDB 3 write/read**: send a [`TelemetryData`] packet via [`send_message`] and then query InfluxDB3 SQL API to verify
//!   the same packet is received.
//! - **MQTT publish/subscribe**: Verifies by ensuring telemetry packet JSON is published to `telemetry` topic, received, and
//!     deserialized properly.
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

use crate::{
    can::{
        MCUMainState, MCUWarningLevel, MCUWorkMode, MotorRotateDirection, MotorState, TelemetryData,
    },
    send::{send_message, Reading, INFLUXDB_DATABASE, INFLUXDB_URL, MQTT_HOST, MQTT_PORT},
};

use tokio::time::Duration;

/// Verifies that a telemetry packet was written to InfluxDB.
///
/// Sends a SQL query for the most recent row in the database returned by
/// [`T::topic`] and compares it with `test_packet`.
///
/// Returns `Ok(true)` if the newest row matches `test_packet`,  
/// `Ok(false)` if it does not,  
/// and `Err(..)` on request or deserialization failure.   
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
    let test_packet = TelemetryData {
        apps_travel: 1.0,
        motor_speed: 1.0,
        motor_torque: 1.0,
        max_motor_torque: 1.0,
        motor_direction: MotorRotateDirection::DirectionForward,
        motor_state: MotorState::MotorStateDriving,
        mcu_main_state: MCUMainState::StateRun,
        mcu_work_mode: MCUWorkMode::WorkModeStandby,
        mcu_voltage: 1.0,
        mcu_current: 1.0,
        motor_temp: 1,
        mcu_temp: 1,
        dc_main_wire_over_volt_fault: false,
        dc_main_wire_over_curr_fault: false,
        motor_over_spd_fault: false,
        motor_phase_curr_fault: false,
        motor_stall_fault: false,
        mcu_warning_level: MCUWarningLevel::ErrorNone,
        debug_0: 0.0,
        debug_1: 0.0,
        debug_2: 0.0,
        debug_3: 0.0,
    };

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
    let listener_data: TelemetryData = TelemetryData {
        apps_travel: 0.0,
        motor_speed: 0.0,
        motor_torque: 0.0,
        max_motor_torque: 0.0,
        motor_direction: MotorRotateDirection::DirectionForward,
        motor_state: MotorState::MotorStateIdle,
        mcu_main_state: MCUMainState::StatePowerOff,
        mcu_work_mode: MCUWorkMode::WorkModeStandby,
        mcu_voltage: 0.0,
        mcu_current: 0.0,
        motor_temp: 0,
        mcu_temp: 0,
        dc_main_wire_over_volt_fault: false,
        dc_main_wire_over_curr_fault: false,
        motor_over_spd_fault: false,
        motor_phase_curr_fault: false,
        motor_stall_fault: false,
        mcu_warning_level: MCUWarningLevel::ErrorNone,
        debug_0: 0.0,
        debug_1: 0.0,
        debug_2: 0.0,
        debug_3: 0.0,
    };

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
