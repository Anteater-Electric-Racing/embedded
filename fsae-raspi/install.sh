#!/bin/sh

# install rust
curl https://sh.rustup.rs -sSf | sh -s -- -y

# installing influxdb3
./install_influxdb3.sh

# installing service
sudo ./add_service.sh

# installing network
sudo ./add_network.sh