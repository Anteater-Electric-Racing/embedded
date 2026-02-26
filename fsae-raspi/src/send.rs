use rumqttc::{AsyncClient, MqttOptions, QoS};
use serde::Serialize;
use taos::{AsyncBindable, AsyncQueryable, AsyncTBuilder, ColumnView, Stmt, Taos, TaosBuilder};
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
    fn table() -> &'static str;
    fn stmt_sql() -> &'static str;
    fn column_views(&self) -> Vec<ColumnView>;
}

static TAOS: OnceCell<Taos> = OnceCell::const_new();
static MQTT_CLIENT: OnceCell<AsyncClient> = OnceCell::const_new();

async fn get_taos_client() -> &'static Taos {
    TAOS.get_or_init(|| async {
        match TaosBuilder::from_dsn(TAOS_URL) {
            Ok(builder) => match builder.build().await {
                Ok(taos) => taos,
                Err(e) => {
                    error!(%e, "Failed to build Taos client");
                    panic!("Failed to build Taos client: {e}");
                }
            },
            Err(e) => {
                error!(%e, "Failed to create TaosBuilder from DSN");
                panic!("Failed to create TaosBuilder from DSN: {e}");
            }
        }
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

pub async fn send_message<T: Reading + Send + 'static>(message: T) {
    let json = match serde_json::to_string(&message) {
        Ok(j) => j,
        Err(e) => {
            error!(%e, "Failed to serialize message");
            return;
        }
    };
    let topic = T::topic();
    let sql = T::stmt_sql();
    let cols = message.column_views();

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
                let taos = get_taos_client().await;
                let result: Result<(), Box<dyn std::error::Error>> = async {
                    let mut stmt = Stmt::init(taos).await?;
                    stmt.prepare(sql).await?;
                    stmt.bind(&cols).await?;
                    stmt.add_batch().await?;
                    stmt.execute().await?;
                    Ok(())
                }
                .await;
                if let Err(e) = result {
                    error!(%e, "Failed to insert to TDengine");
                }
            }
        );
    });
}
