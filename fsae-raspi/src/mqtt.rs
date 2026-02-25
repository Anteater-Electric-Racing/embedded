use rumqttd::Broker;
use tracing::error;

pub fn mqttd() {
    let config = match config::Config::builder()
        .add_source(config::File::from_str(
            include_str!("../rumqttd.toml"),
            config::FileFormat::Toml,
        ))
        .build()
    {
        Ok(config) => config,
        Err(e) => {
            error!(?e, "Failed to load MQTT broker config");
            return;
        }
    };

    let config = match config.try_deserialize() {
        Ok(config) => config,
        Err(e) => {
            error!(?e, "Failed to deserialize MQTT broker config");
            return;
        }
    };

    let mut broker = Broker::new(config);
    if let Err(e) = broker.start() {
        error!(?e, "Failed to start MQTT broker");
    }
}
