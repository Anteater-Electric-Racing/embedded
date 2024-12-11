# Architecture
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
# Development Environment
## Devcontainer
- Install the [Devcontainer](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension for VSCode
- Open the project in VSCode
- Open the command palette (Super+Shift+P) and run `Dev Containers: Rebuild and Reopen in Container`
- The project will now be running in a container with all the necessary dependencies (rust) and services (grafana, influxdb)
- Now run `cargo run` to start the logging service
- You can view the dashboard at `http://localhost:3000` (default username: admin, password: admin)