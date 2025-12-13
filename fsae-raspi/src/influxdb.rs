//! Utilities for converting sensor data into InfluxDB line protocol.
//!
//! This module provides a helper function that converts any type implementing
//! [`Reading`] into a InfluxDB line protocol for the `/api/v3/write_lp` endpoint.


use crate::send::Reading;

/// Converts a [`Reading`] into InfluxDB line protocol.
/// 
/// # Line protocol format
///
/// ```text
/// <measurement> field1=value1,field2="value2",field3=1i <timestamp>
/// ```
/// 
/// The measurement name is taken from [`Reading::topic`]. The function
/// serializes `message` to JSON and, if it is a JSON object, converts each
/// key–value pair into a line protocol field:
///  - JSON numbers are written as:
///   - `123i` for integer types
///   - `123.45` for floating-point values
/// - JSON strings are written as `"..."`
/// - JSON booleans are written as `true` or `false`
/// `<timestamp>` is the current UTC time in nanoseconds
/// 
/// # Returns
/// 
/// A `String` containing a single InfluxDB line protocol record. If the value
/// cannot be converted to a JSON object, only the measurement name and timestamp are emitted.
/// 
/// # Examples
///
/// ```no_run
/// use serde::Serialize;
/// use crate::send::Reading;
/// use crate::influxdb::to_line_protocol;
///
/// #[derive(Serialize)]
/// struct Telemetry {
///     apps_travel: f32,
///     motor_speed: f32,
/// }
///
/// impl Reading for Telemetry {
///     fn topic() -> &'static str {
///         "telemetry"
///     }
/// }
///
/// fn example() {
///     let t = Telemetry {
///         apps_travel: 0.5,
///         motor_speed: 1234.0,
///     };
///
///     let lp = to_line_protocol(&t);
///     // Example output: telemetry apps_travel=0.5,motor_speed=1234 1730000000000000000
///     println!("{lp}");
/// }
/// ```
pub fn to_line_protocol<T: Reading>(message: &T) -> String {
    let mut line_protocol = T::topic().to_string();

    if let Ok(serde_json::Value::Object(obj)) = serde_json::to_value(message) {
        line_protocol.push(' ');
        let mut first_field = true;

        for (key, val) in obj {
            let field_value = match val {
                serde_json::Value::Number(n) => {
                    if n.is_i64() || n.is_u64() {
                        format!("{}i", n)
                    } else {
                        n.to_string()
                    }
                }
                serde_json::Value::String(s) => format!("\"{}\"", s),
                serde_json::Value::Bool(b) => if b { "true" } else { "false" }.to_string(),
                _ => continue,
            };

            if !first_field {
                line_protocol.push(',');
            }
            first_field = false;

            line_protocol.push_str(&format!("{}={}", key, field_value));
        }
    }

    if let Some(timestamp) = chrono::Utc::now().timestamp_nanos_opt() {
        line_protocol.push_str(&format!(" {}", timestamp));
    }

    line_protocol
}
