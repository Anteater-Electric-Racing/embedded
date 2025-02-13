use rumqttc::{AsyncClient, MqttOptions, QoS};
use std::time::{Duration, Instant};

pub async fn subscribe(queue: usize, host: String) {
    let mut mqttoptions = MqttOptions::new("subscribe", host, 1883);
    mqttoptions.set_keep_alive(Duration::from_secs(5));

    let (client, mut eventloop) = AsyncClient::new(mqttoptions, queue);
    client
        .subscribe("benchmark", QoS::AtMostOnce)
        .await
        .unwrap();

    let mut counter = 0;
    let mut last_print = Instant::now();

    loop {
        match eventloop.poll().await {
            Ok(_notification) => {
                counter += 1;

                if last_print.elapsed() >= Duration::from_secs(1) {
                    println!("Events per second: {}", counter);
                    counter = 0;
                    last_print = Instant::now();
                }
            }
            Err(e) => {
                eprintln!("MQTT connection error in subscribe: {:?}", e);
                tokio::time::sleep(Duration::from_secs(1)).await;
            }
        }
    }
}
