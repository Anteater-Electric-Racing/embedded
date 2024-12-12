use std::time::Duration;

use iced::futures::{SinkExt, Stream};
use iced::stream;
use iced::widget::{column, text, Column};
use iced::{Center, Subscription};
use rumqttc::{AsyncClient, EventLoop, MqttOptions, QoS};

pub fn main() -> iced::Result {
    iced::application("FSAE Dashboard", Dashboard::update, Dashboard::view).subscription(Dashboard::subscription).run()
}

#[derive(Default)]
struct Dashboard {
    text: String,
}


#[derive(Debug, Clone)]
enum Message {
    Background(Event)
}

enum BackgroundState {
    Disconnected,
    Connected(Connection),
}

struct Connection {
    _client: AsyncClient,
    eventloop: EventLoop,
}

#[derive(Debug, Clone)]
enum Event {
    Message(rumqttc::Event),
}

fn mqtt() -> impl Stream<Item = Event> {
    stream::channel(100, |mut output| async move {
        let mut state = BackgroundState::Disconnected;

        loop {
            match &mut state {
                BackgroundState::Disconnected => {
                    let mut mqttoptions = MqttOptions::new("dashboard", "127.0.0.1", 1883);
                    mqttoptions.set_keep_alive(Duration::from_secs(5));
                    let (_client, eventloop) = AsyncClient::new(mqttoptions, 10);
                    _client
                        .subscribe("fsae_raspi/can/bmsreading1", QoS::AtMostOnce)
                        .await
                        .unwrap();
                    state = BackgroundState::Connected(Connection { _client, eventloop });
                }
                BackgroundState::Connected(connection) => {
                    while let Ok(notification) = connection.eventloop.poll().await {
                        println!("{:?}", notification);
                        output.send(Event::Message(notification)).await.unwrap();
                    }
                    println!("Connection lost");
                    state = BackgroundState::Disconnected;
                }
            }
        }
    })
}

impl Dashboard {
    fn update(&mut self, message: Message) {
        match message {
            Message::Background(event) => {
                self.text = format!("{:?}", event);
            }
        }
    }

    fn view(&self) -> Column<Message> {
        column![
            text(&self.text)
        ]
        .padding(20)
        .align_x(Center)
    }

    fn subscription(&self) -> Subscription<Message> {
        Subscription::run(mqtt).map(Message::Background)
    }
}
