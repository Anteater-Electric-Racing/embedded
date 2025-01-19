/**
 * GPS architecture overview
 * GPS [NEO-6M] -> |UART| SC[Teensy]
 * SC -> |CAN| Pi[RaspPi]
 * Pi -> |MQTT| GPS_APP[fsae-gps app]
 * GPS_APP -> |State| TrackView[Track Canvas] | TelemetryView[Telemetry]
 */

/**
 * GPS module architecture
 * States: GPS readings, time, lap count, sector count
 * Logic Functions: Draw track, update telemetry
 * User Interface: Track view, telemetry view
 */

use iced::widget::{canvas, container, row, Column, text};
use iced::{Element, Length, Theme};
use iced::time::{Duration, Instant};

// State - application data
struct GPSApp {
    // general telemetry
    position: (f64, f64), // current lat and long
    history: Vec<(f64, f64)>, // history of positions
    speed: f32, // current speed
    heading: f32, // current heading

    // lap telemetry
    current_lap_time: Duration, // time of current lap
    lap_times: Vec<Duration>, // time of each lap
    lap_count: u32, // number of laps completed

    // starting position
    start_position: (f64, f64), // starting lat and long
    start_threshold: f64, // distance threshold to start position to count as crossing
}

// Messages - events sent to application
// first, clone the state to avoid mutability
#[derive(Debug, Clone)]
enum Message {
    GpsUpdate(GpsReading), // pull from MQTT subscription
    Tick(Instant), // increment lap time
    NewLap, // create new lap
}

// update logic functions
impl Update for GPSApp {
    // change telemetry data
    fn update_telemetry(&mut self, reading:GpsReading) {
        // update other telemetry
        self.speed = reading.speed;
        self.heading = reading.heading;
        self.timestamp = reading.timestamp;
    }

    // increment lap time for each tick
    Message::Tick(instant) => {
        self.lap_time += Duration::from_millis(10);
    }
}

// track position logic
impl CheckTrackPosition for GPSApp {
    // pull position data (history and position)
    fn update_position(&mut self, reading: GpsReading) {
        // set starting position
        if self.start_position == (0.0, 0.0) {
            self.start_position = (reading.latitude, reading.longitude);
            self.start_threshold = 5.0; // 5 meter threshold
        }

        // update position and check for new lap
        self.position = (reading.latitude, reading.longitude);
        if self.check_track_position() {
            self.handle_new_lap();
        }

        // update history
        self.history.push(self.position);
        if self.history.len() > 1000 {
            self.history.remove(0);
        }
    }

    // check track position
    fn check_track_position(&mut self) {

    }

    // calculate distance between two points
    fn calculate_distance(pos1: (f64, f64), pos2: (f64, f64)) -> f64 {
        
    }

    // create new lap
    fn handle_new_lap(&mut self) {
        // store new lap time
        self.lap_times.push(self.current_lap_time);

        // reset current lap timer
        self.current_lap_time = Duration::from_secs(0);
        self.lap_count += 1;
    }
}

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

    // draw track lines
    // use lat and long pulled from MQTT subscription
    fn draw_track(&mut self, canvas: &mut Canvas) {
        // call track.rs to draw track lines
        TrackMap::draw(canvas, self);
    }

}