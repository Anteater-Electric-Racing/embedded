/*
 * Track visualization
 */
use iced::widget::canvas::{self, Frame, Geometry, Path, Stroke};
use iced::{Point, Rectangle};

pub struct TrackMap {
    position: (f64, f64),
    history: Vec<(f64, f64)>,
}

// creates a new track map
impl TrackMap {
    pub fn new(position: (f64, f64), history: Vec<(f64, f64>)) -> Self {
        Self { position, history }
    }
}

// draw track lines on canvas
impl canvas::Program<Message> for TrackMap {
    fn draw(&self, bounds: Rectangle, _cursor: canvas::Cursor) -> Vec<Geometry> {
        let mut path = Path::new();

        // draw track history
        // windows creates a sliding window 2 points at a time
        for points in self.history.windows(2) {
            // create a path from points
            let path = Path::new(|p| {
                p.move_to(Point::new(points[0].0 as f32, points[0].1 as f32));
                p.line_to(Point::new(points[1].0 as f32, points[1].1 as f32));
            });
            frame.stroke(&path, Stroke::default().with_width(2.0));
        }

        vec![frame.into_geometry()]
    }
}