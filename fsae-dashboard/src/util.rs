use iced::{
    border,
    widget::{container, horizontal_space, row, text, Container, Text},
    Alignment::Center,
    Font,
    Length::Fill,
};

use crate::{pallate, Message};

pub const ICONS: Font = Font::with_name("Material Symbols Rounded");

pub fn icon(unicode: char) -> Text<'static> {
    text(unicode.to_string()).font(ICONS).align_x(Center)
}

pub fn clamped_stepping_function(value: f64, min: f64, max: f64, steps: i16) -> i16 {
    if value < min {
        -1
    } else if value > max {
        steps + 1
    } else {
        let normalized = (value - min) / (max - min);
        (normalized * steps as f64).round() as i16
    }
}

pub fn grid_cell(value: String, label: &str) -> Container<Message> {
    container(
        container(row![
            text(value).size(128),
            text(label).color(pallate::GRAY_500).size(128),
        ])
        .center(Fill),
    )
    .style(|_theme| {
        container::Style::default().border(border::color(pallate::GRAY_500).width(4).rounded(15))
    })
    .height(Fill)
    .width(Fill)
}
