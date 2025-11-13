#[cfg(test)]
mod tests {
    use crate::{
        can::{
            MCUMainState, MCUWarningLevel, MCUWorkMode, MotorRotateDirection, MotorState,
            TelemetryData,
        },
        send::{send_message, Reading, INFLUXDB_DATABASE, INFLUXDB_URL, MQTT_HOST, MQTT_PORT},
    };
    use std::error::Error;
    use tokio::time::Duration;
    use tokio_socketcan_isotp::{IsoTpSocket, StandardId};

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

                    let data_deserialized = match serde_json::from_str::<TelemetryData>(data_string)
                    {
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

    #[cfg(test)]
    async fn send_telemetry_over_isotp(data: &TelemetryData) -> Result<(), Box<dyn Error>> {
        let socket = IsoTpSocket::open(
            "vcan0",
            StandardId::new(0x123).ok_or("Invalid source ID")?,
            StandardId::new(0x321).ok_or("Invalid destination ID")?,
        )?;

        let payload = bincode::serde::encode_to_vec(data, bincode::config::standard())?;

        socket.write_packet(&payload).await?;

        println!("TelemetryData sent over iso-tp ({} bytes)", payload.len());
        Ok(())
    }

    #[tokio::test]
    async fn test_send_telemetry_over_isotp() -> Result<(), Box<dyn Error>> {
        let data = TelemetryData {
            apps_travel: 72.5,
            motor_speed: 3200.0,
            motor_torque: 85.4,
            max_motor_torque: 120.0,
            motor_direction: MotorRotateDirection::DirectionForward,
            motor_state: MotorState::MotorStateDriving,
            mcu_main_state: MCUMainState::StateRun,
            mcu_work_mode: MCUWorkMode::WorkModeTorque,
            mcu_voltage: 13.8,
            mcu_current: 2.4,
            motor_temp: 75,
            mcu_temp: 68,
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

        send_telemetry_over_isotp(&data).await?;
        Ok(())
    }
}
