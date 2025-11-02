use crate::{
    can::{
        MCUMainState, MCUWarningLevel, MCUWorkMode, MotorRotateDirection, MotorState, TelemetryData,
    },
    send::{send_message, Reading, INFLUXDB_DATABASE, INFLUXDB_URL},
};

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
