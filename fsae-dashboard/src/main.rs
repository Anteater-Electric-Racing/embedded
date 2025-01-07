mod mqtt;
mod pallate;
mod util;

use std::time::{Duration, Instant};

use iced::alignment::Horizontal::Right;
use iced::widget::{column, container, horizontal_space, row, stack, text, Stack};
use iced::Length::Fill;
use iced::{border, time, Subscription, Theme};
use iced::{Color, Font};
use util::{clamped_stepping_function, icon};

pub fn main() -> iced::Result {
    iced::application("FSAE Dashboard", Dashboard::update, Dashboard::view)
        .subscription(Dashboard::subscription)
        .font(include_bytes!(
            "../fonts/MaterialSymbolsRounded-Regular.ttf"
        ))
        .default_font(Font::MONOSPACE)
        .theme(Dashboard::theme)
        .run()
}

#[derive(Default)]
struct Dashboard {
    voltage: i16,
    current: i16,
    temp: u8,
    speed: u16,
    stopwatch: Duration,
    best_lap: Duration,
    last_lap: Duration,
}

#[derive(Debug, Clone)]
enum Message {
    Background(mqtt::Event),
    Tick(Instant),
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
            Message::Tick(_instant) => {
                self.stopwatch += Duration::from_millis(10);
            }
        }
    }

    fn view(&self) -> Stack<Message> {
        let voltage_level =
            clamped_stepping_function(self.voltage as f64, util::VOLTAGE_MIN, util::VOLTAGE_MAX, 7);
        let (voltage_color, voltage_icon) = match voltage_level {
            0 => (pallate::RED_500, '\u{ebdc}'),
            1 => (pallate::ORANGE_500, '\u{ebd9}'),
            2 => (pallate::YELLOW_500, '\u{ebe0}'),
            3 => (pallate::GRAY_500, '\u{ebdd}'),
            4 => (pallate::GRAY_500, '\u{ebe2}'),
            5 => (pallate::GRAY_500, '\u{ebd4}'),
            6 => (pallate::GRAY_500, '\u{ebd2}'),
            7 => (pallate::GRAY_500, '\u{e1a4}'),
            _ => (pallate::AMBER_500, '\u{ebdc}'),
        };
        let current_level =
            clamped_stepping_function(self.current as f64, util::CURRENT_MIN, util::CURRENT_MAX, 7);
        let current_color = match current_level {
            0 => pallate::GRAY_500,
            1 => pallate::GRAY_500,
            2 => pallate::GRAY_500,
            3 => pallate::GRAY_500,
            4 => pallate::GRAY_500,
            5 => pallate::GRAY_500,
            6 => pallate::GRAY_500,
            7 => pallate::ORANGE_500,
            _ => pallate::AMBER_500,
        };
        let temp_level =
            clamped_stepping_function(self.temp as f64, util::TEMP_MIN, util::TEMP_MAX, 4);
        let temp_color: Color = match temp_level {
            0 => pallate::CYAN_500,
            1 => pallate::GRAY_500,
            2 => pallate::GRAY_500,
            3 => pallate::YELLOW_500,
            4 => pallate::RED_500,
            _ => pallate::AMBER_500,
        };
        stack![
            row![
                container(row![
                    horizontal_space().width(15),
                    text(format!("{:>2}", self.voltage)).size(50),
                    text("V").size(50).color(pallate::GRAY_500),
                    icon(voltage_icon).size(50).color(voltage_color),
                    horizontal_space().width(5),
                ])
                .style(|_theme| {
                    container::Style::default()
                        .border(border::color(pallate::GRAY_500).width(4).rounded(12))
                }),
                container(row![
                    horizontal_space().width(15),
                    text(format!("{:>3}", self.current)).size(50),
                    text("A").size(50).color(pallate::GRAY_500),
                    icon('\u{ea0b}').size(50).color(current_color),
                    horizontal_space().width(5),
                ])
                .style(|_theme| {
                    container::Style::default()
                        .border(border::color(pallate::GRAY_500).width(4).rounded(15))
                }),
                container(row![
                    horizontal_space().width(15),
                    text(format!("{:>2}", self.temp)).size(50),
                    text("°C").size(50).color(pallate::GRAY_500),
                    icon('\u{e846}').size(50).color(temp_color),
                    horizontal_space().width(5),
                ])
                .style(|_theme| {
                    container::Style::default()
                        .border(border::color(pallate::GRAY_500).width(4).rounded(15))
                }),
                container(row![
                    horizontal_space().width(15),
                    text(format!("{:>2}", self.speed)).size(50),
                    text("mph").size(50).color(pallate::GRAY_500),
                    icon('\u{e9e4}').size(50).color(pallate::GRAY_500),
                    horizontal_space().width(12),
                ])
                .style(|_theme| {
                    container::Style::default()
                        .border(border::color(pallate::GRAY_500).width(4).rounded(15))
                }),
            ]
            .spacing(20)
            .padding(20)
            .wrap(),
            container(
                column![
                    text(format!("{:.2}", self.stopwatch.as_secs_f64())).size(100),
                    text(format!("best lap {:.2}", self.best_lap.as_secs_f64()))
                        .size(30)
                        .color(pallate::GRAY_500),
                    text(format!("last lap {:.2}", self.last_lap.as_secs_f64()))
                        .size(30)
                        .color(pallate::GRAY_500),
                ]
                .align_x(Right)
            )
            .center(Fill),
        ]
        .width(Fill)
        .height(Fill)
    }

    fn theme(&self) -> Theme {
        Theme::CatppuccinMocha
    }

    fn subscription(&self) -> Subscription<Message> {
        Subscription::batch(vec![
            Subscription::run(mqtt::mqtt).map(Message::Background),
            time::every(Duration::from_millis(10)).map(Message::Tick),
        ])
    }
}
