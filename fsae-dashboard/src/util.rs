use iced::{
    border,
    widget::{container, row, stack, text, Container},
    Length::Fill,
};

use crate::{palette, Message};

// pub const ICONS: Font = Font::with_name("Material Symbols Rounded");

// pub fn icon(unicode: char) -> Text<'static> {
//     text(unicode.to_string()).font(ICONS).align_x(Center)
// }

// pub fn clamped_stepping_function(value: f64, min: f64, max: f64, steps: i16) -> i16 {
//     if value < min {
//         -1
//     } else if value > max {
//         steps + 1
//     } else {
//         let normalized = (value - min) / (max - min);
//         (normalized * steps as f64).round() as i16
//     }
// }

pub fn grid_cell<'a>(
    value: String,
    label: &'a str,
    name: &'a str,
    warning: bool,
    critical: bool,
) -> Container<'a, Message> {
    container(stack![
        container(row![
            text(value).size(128).color(if critical {
                palette::ROSE_500
            } else if warning {
                palette::AMBER_500
            } else {
                palette::GRAY_500
            }),
            text(label).color(palette::GRAY_500).size(128),
        ])
        .center(Fill),
        container(text(name).color(palette::GRAY_500).size(48)).center_x(Fill)
    ])
    .style(|_theme| {
        container::Style::default().border(border::color(palette::GRAY_500).width(4).rounded(15))
    })
    .height(Fill)
    .width(Fill)
}
