use serde_json::Value;
use crate::send::{Reading, INFLUXDB_URL, INFLUXDB_DATABASE, INFLUX_CLIENT};

// verify if test packet was sent to influxdb3
// returns 'true' if a matching packet is found and 'false' otherwise.
pub async fn verify_influx_write<T: Reading + serde::Serialize>(
    test_packet: &T,
) -> bool {
    // build query string - topic is "telemetry"
    let query = format!("SELECT * FROM {} ORDER BY time DESC LIMIT 1", T::topic());
    let url = format!("{}/api/v3/query_sql?db={}&format=json", INFLUXDB_URL, INFLUXDB_DATABASE);

    let body = serde_json::json!({
        "db": INFLUXDB_DATABASE,
        "q": query,
        "format": "json"
    });

    let resp = INFLUX_CLIENT
        .post(&url)
        .header("Authorization", "Bearer apiv3_TQdSxXbtRc8qbzb4ejQOa-ir9-deb4fSVe5Lc-RgvQZqPKikusEJtZpQmEJakPtxZvst8wW4B20KB8iSGLC-Tg")
        .header("Content-Type", "application/json")
        .body(body.to_string())
        .send()
        .await;

    let resp = match resp {
        Ok(r) => r,
        Err(e) => {
            eprintln!("Failed to query InfluxDB: {}", e);
            return false;
        }
    };

    //reads raw text 
    let text = match resp.text().await {
        Ok(t) => t,
        Err(e) => {
            eprintln!("Failed to read InfluxDB response text: {}", e);
            return false;
        }
    };

    println!("InfluxDB raw response: {}", text);

    //parse json to serde_json
    let json: serde_json::Value = match serde_json::from_str(&text) {
        Ok(j) => j,
        Err(e) => {
            eprintln!("Failed to deserialize InfluxDB JSON response: {}", e);
            return false;
        }
    };

    // serialize test packet to json to compare 
    let Ok(packet_json) = serde_json::to_value(test_packet) else {
        eprintln!("Failed to serialize test packet for comparison");
        return false;
    };

    // goes through rows to compare test packet
    if let Some(arr) = json.as_array() {
        for row in arr {
            if let Some(row_obj) = row.as_object() {
                let mut matched = true;

                // compare only the overlapping fields that match
                for (k, v) in packet_json.as_object().unwrap() {
                    if let Some(db_val) = row_obj.get(k) {
                        if db_val.to_string() != v.to_string() {
                            matched = false;
                            break;
                        }
                    }
                }

                if matched {
                    println!("Matching packet found in InfluxDB");
                    return true;
                }
            }
        }
    }

    println!("No matching packet found in InfluxDB");
    return false;
}
