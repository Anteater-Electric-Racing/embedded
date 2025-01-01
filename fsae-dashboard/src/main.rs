mod mqtt;
mod pallate;
mod util;

use iced::alignment::Horizontal::Right;
use iced::widget::{center, column, row, stack, text, Stack};
use iced::Length::Fill;
use iced::Subscription;
use iced::{Color, Font};
use util::{clamped_stepping_function, icon};

pub fn main() -> iced::Result {
    iced::application("FSAE Dashboard", Dashboard::update, Dashboard::view)
        .subscription(Dashboard::subscription)
        .font(include_bytes!(
            "../fonts/MaterialSymbolsRounded-Regular.ttf"
        ))
        .default_font(Font::MONOSPACE)
        .run()
}

#[derive(Default)]
struct Dashboard {
    voltage: i16,
    current: i16,
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
                    self.speed = (rpm as f64 * util::SPEED_TO_RPM) as u16;
                }
            }
        }
    }

    fn view(&self) -> Stack<Message> {
        let voltage_level =
            clamped_stepping_function(self.voltage as f64, util::VOLTAGE_MIN, util::VOLTAGE_MAX, 7);
        let (voltage_color, voltage_icon) = match voltage_level {
            0 => (pallate::RED_500, '\u{ebdc}'),    // Red
            1 => (pallate::ORANGE_500, '\u{ebd9}'), // Orange
            2 => (pallate::YELLOW_500, '\u{ebe0}'), // Yellow
            3 => (pallate::LIME_500, '\u{ebdd}'),   // Lime
            4 => (pallate::GREEN_500, '\u{ebe2}'),  // Green
            5 => (pallate::GREEN_700, '\u{ebd4}'),  // DarkGreen
            6 => (pallate::CYAN_500, '\u{ebd2}'),   // Cyan
            7 => (pallate::BLUE_500, '\u{e1a4}'),   // Blue
            _ => (Color::WHITE, '\u{ebdc}'),        // White
        };
        let current_level =
            clamped_stepping_function(self.current as f64, util::CURRENT_MIN, util::CURRENT_MAX, 3);
        let current_color: Color = match current_level {
            0 => pallate::GREEN_500,  // Green
            1 => Color::WHITE,        // White
            2 => pallate::YELLOW_500, // Yellow
            3 => pallate::RED_500,    // Red
            _ => Color::WHITE,        // White
        };
        let temp_level =
            clamped_stepping_function(self.temp as f64, util::TEMP_MIN, util::TEMP_MAX, 4);
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
                    text(format!("{:>3}", self.voltage)).size(50),
                    text("V").size(50).color(pallate::GRAY_500),
                    icon(voltage_icon).size(50).color(voltage_color),
                ],
                row![
                    text(format!("{:>3}", self.current)).size(50),
                    text("A").size(50).color(pallate::GRAY_500),
                    icon('\u{ea0b}').size(50).color(current_color),
                ],
                row![
                    text(format!("{:>3}", self.temp)).size(50),
                    text("°C").size(50).color(pallate::GRAY_500),
                    icon('\u{e1ff}').size(50).color(temp_color),
                ],
            ]
            .spacing(20)
            .padding(20),
            center(
                column![
                    text(format!("{:>2}", self.speed))
                        .size(100)
                        .color(Color::WHITE),
                    text("MPH").size(50).color(pallate::GRAY_500)
                ]
                .align_x(Right)
            )
        ]
        .width(Fill)
        .height(Fill);
        height
    }

    fn subscription(&self) -> Subscription<Message> {
        Subscription::run(mqtt::mqtt).map(Message::Background)
    }
}
