use std::time::Duration;

use iced::futures::{SinkExt, Stream};
use iced::widget::{column, row, text, Column, Text};
use iced::{stream, Color, Font};
use iced::{Center, Subscription};
use rumqttc::{AsyncClient, EventLoop, MqttOptions, QoS};

pub fn main() -> iced::Result {
    iced::application("FSAE Dashboard", Dashboard::update, Dashboard::view)
        .subscription(Dashboard::subscription)
        .font(include_bytes!(
            "../fonts/MaterialSymbolsRounded-Regular.ttf"
        ))
        .default_font(Font::MONOSPACE)
        .run()
}

const ICONS: Font = Font::with_name("Material Symbols Rounded");

fn icon(unicode: char) -> Text<'static> {
    text(unicode.to_string())
        .font(ICONS)
        .size(50)
        .align_x(Center)
}

fn battery_char(level: usize) -> char {
    match level {
        0 => '\u{ebdc}',
        1 => '\u{ebd9}',
        2 => '\u{ebe0}',
        3 => '\u{ebdd}',
        4 => '\u{ebe2}',
        5 => '\u{ebd4}',
        6 => '\u{ebd2}',
        7 => '\u{e1a4}',
        _ => '\u{ebdc}',
    }
}

#[derive(Default)]
struct Dashboard {
    current: i16,
    voltage: i16,
}

#[derive(Debug, Clone)]
enum Message {
    Background(Event),
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
                    let Ok(_) = _client
                        .subscribe("fsae_raspi/can/bmsreading1", QoS::AtMostOnce)
                        .await
                    else {
                        eprintln!("Failed to subscribe to topic");
                        continue;
                    };
                    state = BackgroundState::Connected(Connection { _client, eventloop });
                }
                BackgroundState::Connected(connection) => {
                    while let Ok(notification) = connection.eventloop.poll().await {
                        println!("{:?}", notification);
                        if let Err(e) = output.send(Event::Message(notification)).await {
                            eprintln!("Failed to send message: {}", e);
                            break;
                        }
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
                let Event::Message(rumqttc::Event::Incoming(rumqttc::Packet::Publish(publish))) =
                    event
                else {
                    return;
                };
                let payload = String::from_utf8_lossy(&publish.payload);
                let Ok(json) = serde_json::from_str::<serde_json::Value>(&payload) else {
                    eprintln!("Failed to parse JSON: {}", payload);
                    return;
                };
                print!("{:?}", json);
                if publish.topic == "fsae_raspi/can/bmsreading1" {
                    let Some(current) = json["current"].as_u64() else {
                        eprintln!("Missing or invalid 'current' field in JSON: {:?}", json);
                        return;
                    };
                    let Some(voltage) = json["inst_voltage"].as_u64() else {
                        eprintln!(
                            "Missing or invalid 'inst_voltage' field in JSON: {:?}",
                            json
                        );
                        return;
                    };
                    self.current = current as i16;
                    self.voltage = voltage as i16;
                }
            }
        }
    }

    fn view(&self) -> Column<Message> {
        let voltage_level = if self.voltage <= 70 {
            0
        } else if self.voltage > 90 {
            7
        } else {
            ((self.voltage - 70) as f64 / 20.0 * 7.0).round() as usize
        };
        let voltage_color: (u8, u8, u8) = match voltage_level {
            0 => (255, 0, 0),     // Red
            1 => (255, 165, 0),   // Orange
            2 => (255, 255, 0),   // Yellow
            3 => (173, 255, 47),  // GreenYellow
            4 => (0, 255, 0),     // Green
            5 => (0, 128, 0),     // DarkGreen
            6 => (0, 255, 255),   // Cyan
            7 => (0, 0, 255),     // Blue
            _ => (255, 255, 255), // White
        };
        let current_level = if self.current < 0 {
            0
        } else if self.current == 0 {
            1
        } else if self.current <= 100 {
            2
        } else {
            3
        };
        let current_color: (u8, u8, u8) = match current_level {
            0 => (0, 255, 0),     // Green
            1 => (255, 255, 255), // White
            2 => (255, 255, 0),   // Yellow
            3 => (255, 0, 0),     // Red
            _ => (255, 255, 255), // White
        };
        column![row![
            row![
                text(format!("{}V", self.voltage))
                    .size(30)
                    .color(Color::from_rgb8(
                        voltage_color.0,
                        voltage_color.1,
                        voltage_color.2
                    )),
                icon(battery_char(voltage_level))
                    .size(30)
                    .color(Color::from_rgb8(
                        voltage_color.0,
                        voltage_color.1,
                        voltage_color.2
                    )),
            ]
            .padding(20),
            row![
                text(format!("{:>3}A", self.current))
                    .size(30)
                    .color(Color::from_rgb8(
                        current_color.0,
                        current_color.1,
                        current_color.2
                    )),
                icon('\u{ea0b}').size(30).color(Color::from_rgb8(
                    current_color.0,
                    current_color.1,
                    current_color.2
                )),
            ]
            .padding(20)
        ]]
    }

    fn subscription(&self) -> Subscription<Message> {
        Subscription::run(mqtt).map(Message::Background)
    }
}
