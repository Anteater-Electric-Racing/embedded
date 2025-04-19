use socketcan_isotp::{self, IsoTpSocket, StandardId};

const SENSOR_PIN_AMT_ADC0: usize = 8;
const SENSOR_PIN_AMT_ADC1: usize = 8;

#[derive(Debug)]
struct TelemetryData {
    accumulator_voltage: f32,
    accumulator_temp: f32,

    tractive_system_voltage: f32,
    adc_0_reads: [u16; SENSOR_PIN_AMT_ADC0],
    adc_1_reads: [u16; SENSOR_PIN_AMT_ADC1],
}

fn extract_adc_reads(slice: &[u8], count: usize) -> [u16; SENSOR_PIN_AMT_ADC0] {
    let mut reads = [0u16; SENSOR_PIN_AMT_ADC0];
    for i in 0..count {
        if i * 2 + 1 < slice.len() {
            reads[i] = u16::from_le_bytes([slice[i * 2], slice[i * 2 + 1]]);
        }
    }
    reads
}

fn main() -> Result<(), socketcan_isotp::Error> {
    println!("Starting socketcan_isotp example");
    let mut tp_socket = IsoTpSocket::open(
        "can0",
        StandardId::new(0x666).expect("Invalid src id"),
        StandardId::new(0x777).expect("Invalid dst id"),
    )?;

    loop {
        let buffer = tp_socket.read()?;
        println!("read {} bytes", buffer.len());

        print!("    Buffer: ");
        for x in buffer {
            print!("{:X?} ", x);
        }
        println!("");

        if buffer.len() < 44 {
            println!("    Buffer too small to construct TelemetryData");
            continue;
        }
        let t = TelemetryData {
            accumulator_voltage: f32::from_le_bytes(buffer[0..4].try_into().unwrap()),
            accumulator_temp: f32::from_le_bytes(buffer[4..8].try_into().unwrap()),
            tractive_system_voltage: f32::from_le_bytes(buffer[8..12].try_into().unwrap()),
            adc_0_reads: extract_adc_reads(&buffer[12..28], SENSOR_PIN_AMT_ADC0),
            adc_1_reads: extract_adc_reads(&buffer[28..44], SENSOR_PIN_AMT_ADC1),
        };
        println!("    TelemetryData: {:#?}", t);
    }
}
