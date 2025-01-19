use std::time::Duration;

use iced::widget::{canvas, container, button, text};
use iced::widget::canvas::{stroke, Cache, Geometry, Path, Stroke};
use iced::{
    Degrees, Size, Fill, Element, Point, Rectangle, Renderer, Subscription, Theme, mouse, Pixels, alignment, Font, Radians, Vector
};

mod dummy;
mod pallate;





pub fn main() -> iced::Result {
    iced::application("FSAE Speedometer", Speedometer::update,Speedometer::view)
    .theme(Speedometer::theme)
    .subscription(Speedometer::subscription)
    .run()
}




#[derive(Debug, Clone)]
enum Message {
    GetSpeed,
}

struct Speedometer {
    speed: u32,
    speedometer: Cache,
}

impl Speedometer {

    fn update(&mut self, message: Message) {
        match message {
            Message::GetSpeed => {
                /*let speed = dummy::dummy(); // Simulate fetching speed
                if self.speed != speed {
                    self.speed = speed;
                    self.speedometer.clear(); // Clear the cache so the view updates
                } */
                 println!("Speed: {} mph", self.speed);
                 if self.speed < 140 {
                    self.speed +=1;
                 } else {
                     self.speed = 0;
                 }
                 self.speedometer.clear();

            }
        }
    }



    fn view(&self) -> Element<Message> {
        let canvas = canvas(self as &Self).width(Fill).height(Fill);

        container(
            canvas
        ).padding(20).into()

    }
    fn theme(&self) -> Theme {
        Theme::CatppuccinMocha
    }
    fn subscription(&self) -> Subscription<Message> {
        iced::time::every(Duration::from_millis(10)).map(|_| Message::GetSpeed)
    }


}

impl Default for Speedometer {
    fn default() -> Self {
        Self {
            speed: 0,
            speedometer: Cache::default(),
        }
    }
}

impl<Message> canvas::Program<Message> for Speedometer {
    type State = ();

    fn draw(
        &self,
        _state: &Self::State,
        renderer: &Renderer,
        theme: &Theme,
        bounds: Rectangle,
        _cursor: mouse::Cursor,
    ) -> Vec<Geometry> {
        let speedometer = self.speedometer.draw(renderer, bounds.size(), |frame| {
            let palette = theme.extended_palette();

            let center = frame.center();
            let radius = frame.width().min(frame.height()) / 2.0;

            // Draw the circular background
            let background = Path::circle(center, radius);
            frame.fill(&background, palette.secondary.strong.color);

            // Draw the current speed in the center
            frame.fill_text(iced::widget::canvas::Text {
                content: format!("{} mph", self.speed),
                position: Point::new(center.x, center.y + radius / 3.0),
                size: iced::Pixels(radius / 5.0),
                color: palette.secondary.strong.text,
                horizontal_alignment: iced::alignment::Horizontal::Center,
                vertical_alignment: iced::alignment::Vertical::Center,
                ..Default::default()
            });

            frame.translate(Vector::new(center.x, center.y));

            // Draw the speedometer needle
            let needle_angle = hand_rotation(self.speed, 140);
            frame.with_save(|frame| {
                frame.rotate(needle_angle);
                let needle = Path::line(
                    Point::new(0.0, -0.2 * radius),
                    Point::new(0.0, -0.8 * radius),
                );
                frame.stroke(&needle, Stroke {
                    style: stroke::Style::Solid(palette.secondary.strong.text),
                    width: 10.0,
                    ..Default::default()
                });
            });


            // Draw speedometer numbers
            for spd in (0..=140).step_by(20) {
                let angle = Radians::from(hand_rotation(spd, 160)) - Radians::from(Degrees(45.0));
                let x = -radius * angle.0.cos();
                let y = -radius * angle.0.sin();

                if spd != 140 {
                    frame.fill_text(canvas::Text {
                        content: format!("{}", spd),
                        size: (radius / 5.0).into(),
                        position: Point::new(x * 0.75, y * 0.75),
                        color: palette.secondary.strong.text,
                        horizontal_alignment: alignment::Horizontal::Center,
                        vertical_alignment: alignment::Vertical::Center,
                        font: Font::MONOSPACE,
                        ..canvas::Text::default()
                    });
                }

            }

            // Draw the speedometer ticks
            for tick in 0..16 {
                let angle = hand_rotation(tick, 16);
                let width = if (tick % 2) == 0 { 5.0 } else { 3.0 };
                let height = if (tick % 2) == 0 { 20.0 } else { 10.0 };

                frame.with_save(|frame| {
                    frame.rotate(angle);
                    frame.fill(
                        &Path::rectangle(
                            Point::new(0.0, radius - 15.0),
                            Size::new(width, height),
                        ),
                        palette.secondary.strong.text,
                    );
                });
            }
        });

        vec![speedometer]
    }

}

fn hand_rotation(n: u32, total: u32) -> Degrees {
    let turns = n as f32 / total as f32;

    Degrees(360.0 * turns)
}
