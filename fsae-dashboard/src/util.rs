use iced::{widget::{text, Text}, Alignment::Center, Font};

pub const VOLTAGE_MIN: f64 = 70.0;
pub const VOLTAGE_MAX: f64 = 90.0;
pub const CURRENT_MIN: f64 = 0.0;
pub const CURRENT_MAX: f64 = 250.0;
pub const TEMP_MIN: f64 = 0.0;
pub const TEMP_MAX: f64 = 50.0;
pub const SPEED_TO_RPM: f64 = 60.0 / 6000.0;
pub const ICONS: Font = Font::with_name("Material Symbols Rounded");

pub fn icon(unicode: char) -> Text<'static> {
    text(unicode.to_string()).font(ICONS).align_x(Center)
}

pub fn clamped_stepping_function(value: f64, min: f64, max: f64, steps: usize) -> usize {
    if value < min {
        0
    } else if value > max {
        steps
    } else {
        let normalized = (value - min) / (max - min);
        (normalized * steps as f64).round() as usize
    }
}
