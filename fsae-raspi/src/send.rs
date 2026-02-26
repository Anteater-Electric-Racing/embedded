use rumqttc::{AsyncClient, MqttOptions, QoS};
use serde::Serialize;
use taos::{AsyncQueryable, AsyncTBuilder, Pool, Taos, TaosBuilder};
use tokio::time::Duration;
use tracing::error;

use tokio::sync::OnceCell;

pub const TAOS_URL: &str = "taos://localhost:6030";
pub const TAOS_DATABASE: &str = "fsae";

pub const MQTT_ID: &str = "fsae";
pub const MQTT_HOST: &str = "127.0.0.1";
pub const MQTT_PORT: u16 = 1883;

pub trait Reading: Serialize {
    fn topic() -> &'static str;
    fn insert_sql(&self) -> String;
}

static TAOS: OnceCell<Taos> = OnceCell::const_new();
static MQTT_CLIENT: OnceCell<AsyncClient> = OnceCell::const_new();

async fn get_taos_client() -> &'static Taos {
    TAOS.get_or_init(|| async {
        let x = TaosBuilder::from_dsn(TAOS_URL)
            .unwrap()
            .build()
            .await
            .unwrap();
        x.exec(&format!(
        "CREATE DATABASE IF NOT EXISTS {db}; \
            USE {db}; \
            CREATE STABLE IF NOT EXISTS telemetry (ts TIMESTAMP, apps_travel FLOAT, motor_speed FLOAT, motor_torque FLOAT, max_motor_torque FLOAT, motor_direction TINYINT, motor_state TINYINT, mcu_main_state TINYINT, mcu_work_mode TINYINT, mcu_voltage FLOAT, mcu_current FLOAT, motor_temp INT, mcu_temp INT, dc_main_wire_over_volt_fault BOOL, dc_main_wire_over_curr_fault BOOL, motor_over_spd_fault BOOL, motor_phase_curr_fault BOOL, motor_stall_fault BOOL, mcu_warning_level TINYINT, over_current BOOL, under_voltage BOOL, over_temperature BOOL, apps BOOL, bse BOOL, bpps BOOL, apps_brake_plaus BOOL, low_battery_voltage BOOL) TAGS (source NCHAR(16));",
            db = crate::send::TAOS_DATABASE
        )).await.unwrap();
        x
    })
    .await
}

async fn get_mqtt_client() -> &'static AsyncClient {
    MQTT_CLIENT
        .get_or_init(|| async {
            let mut mqttoptions = MqttOptions::new(MQTT_ID, MQTT_HOST, MQTT_PORT);
            mqttoptions.set_keep_alive(Duration::from_secs(5));
            let (mqtt_client, mut eventloop) = AsyncClient::new(mqttoptions, 10);

            tokio::spawn(async move {
                loop {
                    if let Err(e) = eventloop.poll().await {
                        error!(%e, "MQTT eventloop error");
                        tokio::time::sleep(Duration::from_secs(1)).await;
                    }
                }
            });

            mqtt_client
        })
        .await
}

pub async fn send_message<T: Reading>(message: T) {
    let json = match serde_json::to_string(&message) {
        Ok(j) => j,
        Err(e) => {
            error!(%e, "Failed to serialize message");
            return;
        }
    };
    let topic = T::topic();
    let sql = message.insert_sql();

    tokio::spawn(async move {
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
                // <-- fix this block
                if let Err(e) = get_taos_client().await.exec(&sql).await {
                    error!(%e, "Failed to insert to TDengine");
                }
            }
        );
    });
}
