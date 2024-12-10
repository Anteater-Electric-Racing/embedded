```mermaid
flowchart TD
    B(Brake Sensors) -->|Analog| A(["<a href='https://github.com/AlistairKeiller/FSAE/tree/master/fsae-arduino' target='_blank'>Arduino</a>"])
    L(Linear Potentiometers) -->|Analog| A
    A -->|UART| Pi
    O(Orion BMS) -->|CAN| Pi
    K(Kelly Motor Controllers) -->|CAN| Pi
    IMS(Elcon Charger) -->|CAN| Pi
    subgraph Pi[Raspberry Pi System]
        R(["<a href='https://github.com/AlistairKeiller/FSAE/tree/master/fsae-raspi' target='_blank'>Rust Logging Code</a>"]) -->|HTTP| I(InfluxDB)
    end
    subgraph Graphana[Wireless Dashboard]
        I -->|HTTP| F(Full Histroy Preview)
        R -->|MQTT| P(Real Time Preview)
    end
```
