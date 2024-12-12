mod mqtt;
mod pallate;

use iced::widget::{center, row, stack, text, Stack, Text};
use iced::Length::Fill;
use iced::{Center, Subscription};
use iced::{Color, Font};

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
    text(unicode.to_string()).font(ICONS).align_x(Center)
}

#[derive(Default)]
struct Dashboard {
    current: i16,
    voltage: i16,
    temp: u8,
    speed: u16,
}

#[derive(Debug, Clone)]
enum Message {
    Background(mqtt::Event),
}

impl Dashboard {
    fn update(&mut self, message: Message) {
        match message {
            Message::Background(event) => {
                let mqtt::Event::Message(rumqttc::Event::Incoming(rumqttc::Packet::Publish(
                    publish,
                ))) = event
                else {
                    return;
                };
                let payload = String::from_utf8_lossy(&publish.payload);
                let Ok(json) = serde_json::from_str::<serde_json::Value>(&payload) else {
                    eprintln!("Failed to parse JSON: {}", payload);
                    return;
                };
                if publish.topic == "fsae_raspi/can/bmsreading1" {
                    let Some(current) = json["current"].as_u64() else {
                        eprintln!("Missing or invalid 'current' field in JSON: {:?}", json);
                        return;
                    };
                    self.current = current as i16;
                    let Some(voltage) = json["inst_voltage"].as_u64() else {
                        eprintln!(
                            "Missing or invalid 'inst_voltage' field in JSON: {:?}",
                            json
                        );
                        return;
                    };
                    self.voltage = voltage as i16;
                }
                if publish.topic == "fsae_raspi/can/bmsreading2" {
                    let Some(temp) = json["high_temp"].as_u64() else {
                        eprintln!("Missing or invalid 'temp' field in JSON: {:?}", json);
                        return;
                    };
                    self.temp = temp as u8;
                }
                if publish.topic == "fsae_raspi/can/leftescreading1" {
                    let Some(rpm) = json["speed_rpm"].as_u64() else {
                        eprintln!("Missing or invalid 'speed' field in JSON: {:?}", json);
                        return;
                    };
                    self.speed = rpm as u16;
                }
            }
        }
    }

    fn view(&self) -> Stack<Message> {
        let voltage_level = if self.voltage <= 70 {
            0
        } else if self.voltage > 90 {
            7
        } else {
            ((self.voltage - 70) as f64 / 20.0 * 7.0).round() as usize
        };
        let voltage_color: Color = match voltage_level {
            0 => pallate::RED_500,    // Red
            1 => pallate::ORANGE_500, // Orange
            2 => pallate::YELLOW_500, // Yellow
            3 => pallate::LIME_500,   // GreenYellow
            4 => pallate::GREEN_500,  // Green
            5 => pallate::GREEN_700,  // DarkGreen
            6 => pallate::CYAN_500,   // Cyan
            7 => pallate::BLUE_500,   // Blue
            _ => Color::WHITE,        // White
        };
        let voltage_icon = match voltage_level {
            0 => '\u{ebdc}',
            1 => '\u{ebd9}',
            2 => '\u{ebe0}',
            3 => '\u{ebdd}',
            4 => '\u{ebe2}',
            5 => '\u{ebd4}',
            6 => '\u{ebd2}',
            7 => '\u{e1a4}',
            _ => '\u{ebdc}',
        };
        let current_level = if self.current < 0 {
            0
        } else if self.current == 0 {
            1
        } else if self.current <= 250 {
            2
        } else {
            3
        };
        let current_color: Color = match current_level {
            0 => pallate::GREEN_500,  // Green
            1 => Color::WHITE,        // White
            2 => pallate::YELLOW_500, // Yellow
            3 => pallate::RED_500,    // Red
            _ => Color::WHITE,        // White
        };
        let temp_level = if self.temp < 20 {
            0
        } else if self.temp < 30 {
            1
        } else if self.temp < 40 {
            2
        } else if self.temp < 50 {
            3
        } else {
            4
        };
        let temp_color: Color = match temp_level {
            0 => pallate::CYAN_500,   // Cyan
            1 => pallate::BLUE_500,   // Blue
            2 => pallate::GREEN_500,  // Green
            3 => pallate::YELLOW_500, // Yellow
            4 => pallate::RED_500,    // Red
            _ => Color::WHITE,        // White
        };
        let height = stack![
            row![
                row![
                    text(format!("{}V", self.voltage))
                        .size(50)
                        .color(voltage_color),
                    icon(voltage_icon).size(50).color(voltage_color),
                ],
                row![
                    text(format!("{:>3}A", self.current))
                        .size(50)
                        .color(current_color),
                    icon('\u{ea0b}').size(50).color(current_color),
                ],
                row![
                    text(format!("{}°C", self.temp)).size(50).color(temp_color),
                    icon('\u{e1ff}').size(50).color(temp_color),
                ]
            ]
            .spacing(20)
            .padding(20),
            center(text("50 MPH").size(100).color(Color::WHITE))
        ]
        .width(Fill)
        .height(Fill);
        height
    }

    fn subscription(&self) -> Subscription<Message> {
        Subscription::run(mqtt::mqtt).map(Message::Background)
    }
}
