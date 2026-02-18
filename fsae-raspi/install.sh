#!/bin/sh

# influxdb
curl https://www.influxdata.com/d/install_influxdb3.sh | bash

# grafana
sudo apt install -y apt-transport-https software-properties-common wget
sudo mkdir -p /etc/apt/keyrings/
wget -q -O - https://apt.grafana.com/gpg.key | gpg --dearmor | sudo tee /etc/apt/keyrings/grafana.gpg > /dev/null
if ! grep -q "^deb .*\[signed-by=/etc/apt/keyrings/grafana.gpg\] https://apt.grafana.com stable main" /etc/apt/sources.list.d/grafana.list 2>/dev/null; then
    echo "deb [signed-by=/etc/apt/keyrings/grafana.gpg] https://apt.grafana.com stable main" | sudo tee -a /etc/apt/sources.list.d/grafana.list
fi
sudo apt-get update && sudo apt-get install -y grafana
sudo service grafana-server start

# rust
curl https://sh.rustup.rs -sSf | sh -s -- -y

# installing service
sudo ./add_service.sh

# installing network
sudo ./add_network.sh