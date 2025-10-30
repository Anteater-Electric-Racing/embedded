use rumqttc::{AsyncClient, Event, MqttOptions, QoS};
use std::error::Error;
use std::time::Duration;
use tokio::{task, time};

#[tokio::main]
async fn main() {
    let mut mqttoptions = MqttOptions::new("can0", "localhost", 1883);
    mqttoptions.set_keep_alive(Duration::from_secs(5));

    let (client, mut eventloop) = AsyncClient::new(mqttoptions, 10);
    client
        .subscribe("telemetry", QoS::AtMostOnce)
        .await
        .unwrap();

    task::spawn(async move {
        for i in 0..10 {
            client
                .publish("telemetry", QoS::AtLeastOnce, false, vec![i; i as usize])
                .await
                .unwrap();
            time::sleep(Duration::from_millis(100)).await;
        }
    });

    while let Ok(notification) = eventloop.poll().await {
        println!("Received = {:?}", notification);

        match notification {
            Event::Incoming(message) => {
                println!("[Incoming]: {:?}", &message);
            }
            Event::Outgoing(message) => {
                println!("[Outgoing]:  {:?}", &message);
            }
        }
    }
}
