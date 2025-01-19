/**
 * GPS architecture overview
 * GPS [NEO-6M] -> |UART| SC[Teensy]
 * SC -> |CAN| Pi[RaspPi]
 * Pi -> |MQTT| GPS_APP[fsae-gps app]
 */

/**
 * GPS module architecture
 * State
 * 
 */

use iced::widget::{canvas, container, row, Column};
use iced::{Element, Length, Theme};

// State - application data
struct GPSApp {
    position: (f64, f64), // current lat and long
    history: Vec<(f64, f64)>, // history of positions
    speed: f32, // current speed
    heading: f32, // current heading
    lap_time: Duration, // time of current lap
    lap_count: u32, // number of laps completed
}

// Messages - events sent to application
// first, clone the state to avoid mutability
#[derive(Debug, Clone)]
enum Message {
    GpsUpdate(GpsReading), // pull from MQTT subscription
    Tick(Instant), // increment lap time
    NewLap, // create new lap
    SectorChange(u8), // 
}

fn main() {
    println!("Hello, world!");
}
 