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
    pub power: i32,         // watts, -1kw-180kw
    pub energy_left: u32,   // watt hours, 0-30kwh
    pub battery_temp: u8,   // celsius, 0-50
    pub motor_temp: u8,     // celsius, 0-50
    pub inverter_temp: u8,  // celsius, 0-50
    pub lap_time: Duration, // seconds, 0-∞
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
                self.power = json["power"].as_i64().unwrap_or(-1) as i32;
                self.energy_left = json["energy_left"].as_u64().unwrap_or(0) as u32;
                self.battery_temp = json["battery_temp"].as_u64().unwrap_or(0) as u8;
                self.motor_temp = json["motor_temp"].as_u64().unwrap_or(0) as u8;
                self.inverter_temp = json["inverter_temp"].as_u64().unwrap_or(0) as u8;
            }
            Message::Tick(_instant) => {
                self.lap_time += Duration::from_millis(10);
            }
        }
    }

    fn view(&self) -> Column<Message> {
        column![
            row![
                grid_cell(
                    self.power.to_string(),
                    " W",
                    "Power",
                    self.power > 120000 || self.power < -500,
                    self.power > 180000 || self.power < -1000
                ),
                grid_cell(
                    self.battery_temp.to_string(),
                    "°C",
                    "Battery Temp",
                    self.battery_temp > 40,
                    self.battery_temp > 50
                ),
            ]
            .spacing(20),
            row![
                grid_cell(
                    self.energy_left.to_string(),
                    " Wh",
                    "Energy Left",
                    self.energy_left < 5000,
                    self.energy_left < 2000
                ),
                grid_cell(
                    self.motor_temp.to_string(),
                    "°C",
                    "Motor Temp",
                    self.motor_temp > 40,
                    self.motor_temp > 50
                ),
            ]
            .spacing(20),
            row![
                grid_cell(
                    format!("{:.2}", self.lap_time.as_secs_f64()),
                    "s",
                    "Lap Time",
                    false,
                    false
                ),
                grid_cell(
                    self.inverter_temp.to_string(),
                    "°C",
                    "Inverter Temp",
                    self.inverter_temp > 40,
                    self.inverter_temp > 50
                ),
            ]
            .spacing(20),
        ]
        .spacing(20)
        .padding(20)
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
