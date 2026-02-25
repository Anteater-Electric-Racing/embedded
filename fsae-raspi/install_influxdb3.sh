#!/bin/bash
cd ~/Documents
git clone https://github.com/influxdata/influxdb
cd influxdb
cargo build --release
