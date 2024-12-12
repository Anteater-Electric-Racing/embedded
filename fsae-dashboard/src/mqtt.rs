use iced::futures::{SinkExt, Stream};
use iced::stream;
use rumqttc::{AsyncClient, EventLoop, MqttOptions, QoS};
use std::time::Duration;

struct Connection {
    _client: AsyncClient,
    eventloop: EventLoop,
}

enum BackgroundState {
    Disconnected,
    Connected(Connection),
}

#[derive(Debug, Clone)]
pub enum Event {
    Message(rumqttc::Event),
}

pub fn mqtt() -> impl Stream<Item = Event> {
    stream::channel(100, |mut output| async move {
        let mut state = BackgroundState::Disconnected;

        loop {
            match &mut state {
                BackgroundState::Disconnected => {
                    let mut mqttoptions = MqttOptions::new("dashboard", "127.0.0.1", 1883);
                    mqttoptions.set_keep_alive(Duration::from_secs(5));
                    let (_client, eventloop) = AsyncClient::new(mqttoptions, 10);
                    if let Err(e) = _client
                        .subscribe("fsae_raspi/can/bmsreading1", QoS::AtMostOnce)
                        .await
                    {
                        eprintln!("Failed to subscribe to topic bmsreading1 {}", e);
                        continue;
                    };
                    if let Err(e) = _client
                        .subscribe("fsae_raspi/can/bmsreading2", QoS::AtMostOnce)
                        .await
                    {
                        eprintln!("Failed to subscribe to topic bmsreading2 {}", e);
                        continue;
                    };
                    if let Err(e) = _client
                        .subscribe("fsae_raspi/can/leftescreading1", QoS::AtMostOnce)
                        .await
                    {
                        eprintln!("Failed to subscribe to topic leftescreading1 {}", e);
                        continue;
                    };
                    state = BackgroundState::Connected(Connection { _client, eventloop });
                }
                BackgroundState::Connected(connection) => {
                    while let Ok(notification) = connection.eventloop.poll().await {
                        if let Err(e) = output.send(Event::Message(notification)).await {
                            eprintln!("Failed to send message: {}", e);
                            break;
                        }
                    }
                    eprintln!("Connection lost");
                    state = BackgroundState::Disconnected;
                }
            }
        }
    })
}
