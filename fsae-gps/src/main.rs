/**
 * GPS architecture overview
 * GPS [NEO-6M] -> |UART| SC[Teensy]
 * SC -> |CAN| Pi[RaspPi]
 * Pi -> |MQTT| GPS_APP[fsae-gps app]
 */

/**
 * GPS module architecture
 * States: GPS readings, time, lap count, sector count
 * Logic Functions: Draw track, update telemetry
 * User Interface: Track view, telemetry view
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

// update logic functions

// user interface functions
//! change UI in the future 
impl GPSApp {
    fn view(&self) -> Element<Message> {
        // create a canvas to store track lines
        let track_view = canvas(TrackMap::new(&self))
            .width(Length::Fill)
            .height(Length::Fill);

        // create a container to store telemetry
        let telemetry = container(
            Column::new()
                .spacing(10)
                .push(text(format!("Speed: {} km/h", self.speed)))
                .push(text(format!("Lap: {}", self.lap_time.as_secs_f32())))
                .push(text(format!("Lap Count: {}", self.lap_count)))
        );

        row![track_view, telemetry].into()
    }

}