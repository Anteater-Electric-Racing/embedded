use rumqttd::{Broker, Config};

pub const MQTT_ID: &str = "raspi";
pub const MQTT_HOST: &str = "127.0.0.1";
pub const MQTT_PORT: u16 = 1883;

pub async fn mqttd() {
    let builder = tracing_subscriber::fmt()
        .pretty()
        .with_line_number(false)
        .with_file(false)
        .with_thread_ids(false)
        .with_thread_names(false)
        .with_env_filter("rumqttd=warn");

    builder
        .try_init()
        .expect("initialized subscriber succesfully");

    let config = config::Config::builder()
        .add_source(config::File::from_str(
            include_str!("../rumqttd.toml"),
            config::FileFormat::Toml,
        ))
        .build()
        .unwrap();

    let config: Config = config.try_deserialize().unwrap();

    let mut broker = Broker::new(config);
    broker.start().unwrap();
}
