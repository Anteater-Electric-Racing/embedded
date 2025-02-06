use iced::futures::{SinkExt, Stream};
use iced::stream;
use rumqttc::{AsyncClient, EventLoop, MqttOptions, QoS};
use std::time::Duration;

struct Connection {
    _client: AsyncClient,
    eventloop: EventLoop,
}

#[derive(Debug, Clone)]
pub enum Event {
    Message(rumqttc::Event),
}

pub fn mqtt() -> impl Stream<Item = Event> {
    stream::channel(100, |mut output| async move {
        let mut state: Option<Connection>= None;

        loop {
            match &mut state {
                None => {
                    let mut mqttoptions = MqttOptions::new("dashboard", "127.0.0.1", 1883);
                    mqttoptions.set_keep_alive(Duration::from_secs(5));
                    let (_client, eventloop) = AsyncClient::new(mqttoptions, 10);
                    if let Err(e) = _client
                        .subscribe("data", QoS::AtMostOnce)
                        .await
                    {
                        eprintln!("Failed to subscribe to data. Error: {}", e);
                        continue;
                    };
                    state = Some(Connection { _client, eventloop });
                }
                Some(connection) => {
                    match connection.eventloop.poll().await {
                        Ok(notification) => {
                            if let Err(e) = output.send(Event::Message(notification)).await {
                                eprintln!("Failed to send message: {}", e);
                            }
                        }
                        Err(e) => {
                            eprintln!("Failed to poll eventloop: {}", e);
                            state = None;
                        }
                    }
                }
            }
        }
    })
}
