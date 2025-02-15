use rumqttd::Broker;

pub fn mqttd() {
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

    let config = match config::Config::builder()
        .add_source(config::File::from_str(
            include_str!("../rumqttd.toml"),
            config::FileFormat::Toml,
        ))
        .build()
    {
        Ok(config) => config,
        Err(e) => {
            eprintln!("Failed to load config: {:?}", e);
            return;
        }
    };

    let config = match config.try_deserialize() {
        Ok(config) => config,
        Err(e) => {
            eprintln!("Failed to deserialize config: {:?}", e);
            return;
        }
    };

    let mut broker = Broker::new(config);
    if let Err(e) = broker.start() {
        eprintln!("Failed to start broker: {:?}", e);
    }
}
