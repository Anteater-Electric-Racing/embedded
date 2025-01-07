# Architecture

```mermaid
flowchart TD
    B(Brake Sensors) -->|Analog| A(["<a href='https://github.com/AlistairKeiller/FSAE/tree/master/fsae-arduino' target='_blank'>Arduino</a>"])
    L(Linear Potentiometers) -->|Analog| A
    A -->|UART| Pi
    O(Orion BMS) -->|CAN| Pi
    K(Omni Inverter) -->|CAN| Pi
    SC(RP2040 Omni Speed Controller) -->|CAN| Pi
    IMU(MPU6050 IMU) -->|I2C| Pi
    GPS(NEO-6M GPS) -->|UART| Pi
    IMS(Elcon Charger) -->|CAN| Pi
    subgraph Pi[Raspberry Pi System]
        R(["<a href='https://github.com/AlistairKeiller/FSAE/tree/master/fsae-raspi' target='_blank'>Raspi Logger</a>"]) -->|HTTP| I(InfluxDB)
        R -->|MQTT| D(["<a href='https://github.com/AlistairKeiller/FSAE/tree/master/fsae-dashboard' target='_blank'>Raspi Dashboard</a>"])
    end
    subgraph Graphana[Wireless Grafana]
        I -->|HTTP| F(Full Histroy Preview)
        R -->|MQTT| P(Real Time Preview)
    end
```

# Development Environment

## fsae-raspi

- Open the project in VSCode
- Install the [Devcontainer](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension for VSCode
- Open the command palette (Super+Shift+P) and run `Dev Containers: Rebuild and Reopen in Container`
- The project will now be running in a container with all the necessary dependencies (rust) and services (grafana, influxdb)
- Now run `cargo run --bin fsae-raspi` to start the logging service
- You can view the dashboard at `http://localhost:3000` (default username: admin, password: admin)
- You can view the influxdb database (`http://localhost:8086`) or mqtt broker (`localhost:1883`), by adding it as a data source in grafana

## fsae-arduino

- Plug in the Arduino
- Open the project in VSCode
- Install the [PlatformIO](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) extension for VSCode
- Open the command palette (Super+Shift+P) and run `PlatformIO: Upload`
- The code will now be running on the Arduino

## fsae-dashboard

- Open the project in VSCode
- Now run `cargo run --bin fsae-dashboard` to start the dashboard
