use crate::send::Reading;

/// Escapes special characters in a line-protocol field key.
fn escape_field_key(s: &str) -> String {
    s.replace(',', r"\,")
        .replace('=', r"\=")
        .replace(' ', r"\ ")
}

/// Escapes special characters in a line-protocol string field value.
fn escape_string_value(s: &str) -> String {
    s.replace('\\', r"\\").replace('"', r#"\""#)
}

/// Converts any [`Reading`] into an InfluxDB line-protocol string.
///
/// Format: `<measurement> <field1>=<val1>,<field2>=<val2> <timestamp_ns>`
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
                serde_json::Value::String(s) => {
                    format!("\"{}\"", escape_string_value(&s))
                }
                serde_json::Value::Bool(b) => if b { "true" } else { "false" }.to_string(),
                _ => continue,
            };

            if !first_field {
                line_protocol.push(',');
            }
            first_field = false;

            line_protocol.push_str(&format!("{}={}", escape_field_key(&key), field_value));
        }
    }

    if let Some(timestamp) = chrono::Utc::now().timestamp_nanos_opt() {
        line_protocol.push_str(&format!(" {}", timestamp));
    }

    line_protocol
}
