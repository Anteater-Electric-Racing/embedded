use rumqttc::{AsyncClient, MqttOptions, QoS};
use std::time::Duration;
use tokio::task;

pub async fn post(queue: usize, playload_size: usize, host: String) {
    let mut mqttoptions = MqttOptions::new("post", host, 1883);
    mqttoptions.set_keep_alive(Duration::from_secs(5));

    let (client, mut eventloop) = AsyncClient::new(mqttoptions, queue);

    tokio::spawn(async move {
        loop {
            if let Err(e) = eventloop.poll().await {
                eprintln!("MQTT connection error in post: {:?}", e);
                tokio::time::sleep(Duration::from_secs(1)).await;
            }
        }
    });

    task::spawn(async move {
        loop {
            client.publish("benchmark", QoS::AtLeastOnce, false, vec![0; playload_size]).await.unwrap();
        }
    });
}
