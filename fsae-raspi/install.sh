#!/bin/sh

# influxdb
wget -q https://repos.influxdata.com/influxdata-archive_compat.key
echo '393e8779c89ac8d958f81f942f9ad7fb82a25e133faddaf92e15b16e6ac9ce4c influxdata-archive_compat.key' | sha256sum -c && cat influxdata-archive_compat.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/influxdata-archive_compat.gpg > /dev/null
echo 'deb [signed-by=/etc/apt/trusted.gpg.d/influxdata-archive_compat.gpg] https://repos.influxdata.com/debian stable main' | sudo tee /etc/apt/sources.list.d/influxdata.list
sudo apt update && sudo apt install -y influxdb
sudo systemctl enable --now influxdb
influx -execute "CREATE DATABASE fsae"

# grafana
sudo apt install -y apt-transport-https software-properties-common wget
sudo mkdir -p /etc/apt/keyrings/
wget -q -O - https://apt.grafana.com/gpg.key | gpg --dearmor | sudo tee /etc/apt/keyrings/grafana.gpg > /dev/null
if ! grep -q "^deb .*\[signed-by=/etc/apt/keyrings/grafana.gpg\] https://apt.grafana.com stable main" /etc/apt/sources.list.d/grafana.list 2>/dev/null; then
    echo "deb [signed-by=/etc/apt/keyrings/grafana.gpg] https://apt.grafana.com stable main" | sudo tee -a /etc/apt/sources.list.d/grafana.list
fi
sudo apt update
sudo apt install -y grafana
sudo systemctl enable --now grafana-server

# rust
curl https://sh.rustup.rs -sSf | sh -s -- -y

# installing service
sudo ./add_service.sh

# installing network
sudo ./add_network.sh