use crate::{
    can::{
        MCUMainState, MCUWarningLevel, MCUWorkMode, MotorRotateDirection, MotorState, TelemetryData,
    },
    send::{send_message, Reading, INFLUXDB_DATABASE, INFLUXDB_URL},
};

use tokio::time::Duration;

pub async fn verify_influx_write<T: Reading + for<'de> serde::Deserialize<'de> + PartialEq>(
    test_packet: T,
) -> Result<bool, Box<dyn std::error::Error>> {
    let client = reqwest::Client::builder()
        .build()
        .expect("Failed to build InfluxDB client");

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

    return Ok(resp_struct == test_packet);
}

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

fn test_mqtt_send() {
    let rt = tokio::runtime::Runtime::new().unwrap();
    rt.block_on(send_message(TelemetryData {
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
    }));
}

async fn mqtt_listener(telemetry_struct: TelemetryData) {
    // Client and event loop setup
    let mut options = rumqttc::MqttOptions::new("telemetry", "localhost", 1883);
    options.set_keep_alive(Duration::from_secs(5));
    let (client, mut event_loop) = rumqttc::AsyncClient::new(options, 10);

    // Subscribe client to topic
    client
        .subscribe("telemetry", rumqttc::QoS::AtLeastOnce)
        .await
        .expect("[subscribe] | Unable to subscribe to the topic.");

    // Repeat poll check loop while event loop has not returned an Err.
    while let Ok(notification) = event_loop.poll().await {
        // Check for Publish messages and extract
        if let rumqttc::Event::Incoming(rumqttc::Packet::Publish(data)) = notification {
            let data_string = str::from_utf8(&data.payload)
                .expect("[string parse] | Failed to convert bytes into data string.");
            let data_deserialized = serde_json::from_str::<TelemetryData>(data_string)
                .expect("[poll] | Failed to deserialize incoming data.");

            if data_deserialized == telemetry_struct {
                client
                    .publish("telemetry", rumqttc::QoS::AtLeastOnce, false, "true")
                    .await
                    .expect("[Result]: Failed to send result to topic.");
                assert!(true);
                // For test purposes
                break;
            } else {
                client
                    .publish("telemetry", rumqttc::QoS::AtLeastOnce, false, "false")
                    .await
                    .expect("[Result]: Failed to send result to topic.");
                assert!(false);
            }
        }
    }
}

#[test]
fn verify_mqtt_listener() {
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

    // Send test packet
    test_mqtt_send();

    // Run verify
    let runtime = tokio::runtime::Runtime::new().expect("Unable to start listener runtime.");
    runtime.block_on(mqtt_listener(listener_data));
}
