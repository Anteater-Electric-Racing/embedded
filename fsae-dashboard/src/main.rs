mod mqtt;
mod palette;
mod util;

use std::time::{Duration, Instant};

use iced::widget::{column, row, Column};
use iced::Font;
use iced::{time, Subscription, Theme};
use util::grid_cell;

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
    pub lap_time: Duration,  // seconds, 0-∞
    pub energy_left_wh: u32, // watt hours, 0-30kwh
    pub power_w: i32,        // watts, -1kw-180kw
    pub power_a: i32,        // amps, -3a-600a
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
                let mqtt::Event::Message(rumqttc::Event::Incoming(rumqttc::Packet::Publish(p))) =
                    event
                else {
                    return;
                };
                let Ok(json) =
                    serde_json::from_str::<serde_json::Value>(&String::from_utf8_lossy(&p.payload))
                else {
                    eprintln!("Invalid JSON: {}", String::from_utf8_lossy(&p.payload));
                    return;
                };
                self.energy_left_wh = json["energy_left_wh"].as_u64().unwrap_or(0) as u32;
                self.power_w = json["power_w"].as_i64().unwrap_or(-1) as i32;
                self.power_a = json["power_a"].as_i64().unwrap_or(-1) as i32;
            }
            Message::Tick(_instant) => {
                self.lap_time += Duration::from_millis(10);
            }
        }
    }

    fn view(&self) -> Column<'_, Message> {
        column![
            row![
                grid_cell(
                    format!("{:.2}", self.lap_time.as_secs_f64()),
                    "s",
                    "Lap Time",
                    false,
                    false
                ),
                grid_cell(
                    self.power_w.to_string(),
                    " W",
                    "Power",
                    self.power_w > 120000 || self.power_w < -500,
                    self.power_w > 180000 || self.power_w < -1000
                ),
            ]
            .spacing(20),
            row![
                grid_cell(
                    self.energy_left_wh.to_string(),
                    " Wh",
                    "Energy Left",
                    self.energy_left_wh < 5000,
                    self.energy_left_wh < 2000
                ),
                grid_cell(
                    self.power_w.to_string(),
                    " A",
                    "Power",
                    self.power_w > 400 || self.power_w < -2,
                    self.power_w > 600 || self.power_w < -3
                ),
            ]
            .spacing(20),
        ]
        .spacing(20)
        .padding(20)
    }

    fn theme(&self) -> Theme {
        Theme::Moonfly
    }

    fn subscription(&self) -> Subscription<Message> {
        Subscription::batch(vec![
            Subscription::run(mqtt::mqtt).map(Message::Background),
            time::every(Duration::from_millis(10)).map(Message::Tick),
        ])
    }
}
