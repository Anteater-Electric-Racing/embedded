use serde_json::Value;
use crate::send::{Reading, INFLUXDB_URL, INFLUXDB_DATABASE, INFLUX_CLIENT};

// verify if test packet was sent to influxdb3
// returns 'true' if a matching packet is found and 'false' otherwise.
pub async fn verify_influx_write<T: Reading + serde::Serialize>(
    test_packet: &T,
) -> bool {
    // build query string - topic is "telemetry"
    let query = format!("SELECT * FROM {} ORDER BY time DESC LIMIT 5", T::topic());
    let url = format!("{}/api/v3/query?db={}", INFLUXDB_URL, INFLUXDB_DATABASE);

    
    let resp = INFLUX_CLIENT
        .post(&url)
        .header("Authorization", "Bearer apiv3_TQdSxXbtRc8qbzb4ejQOa-ir9-deb4fSVe5Lc-RgvQZqPKikusEJtZpQmEJakPtxZvst8wW4B20KB8iSGLC-Tg")
        .body(query)
        .send()
        .await;

    let Ok(resp) = resp else {
        eprintln!("Failed to query InfluxDB: {}", resp.err().unwrap());
        return false;
    };

    //reads raw text 
    let Ok(text) = resp.text().await else {
        eprintln!("Failed to parse InfluxDB response text");
        return false;
    };

    //parse json to serde_json
    let Ok(json): Result<Value, _> = serde_json::from_str(&text) else {
        eprintln!("Failed to deserialize InfluxDB JSON response");
        return false;
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
