#!/bin/bash
set -e

if [ "$(id -u)" -ne 0 ]; then
    echo "Please run as root (sudo ./install.sh)" >&2
    exit 1
fi

REAL_USER="${SUDO_USER:-$(logname)}"
REAL_HOME=$(eval echo "~$REAL_USER")

echo "==> Installing Rust for $REAL_USER..."
sudo -u "$REAL_USER" bash -c 'curl https://sh.rustup.rs -sSf | sh -s -- -y'

echo "==> Building InfluxDB3..."
sudo -u "$REAL_USER" bash -c "
    cd '$REAL_HOME/Documents' &&
    git clone --depth=1 https://github.com/influxdata/influxdb || true &&
    cd influxdb &&
    '$REAL_HOME/.cargo/bin/cargo' build --release
"

echo "==> Installing InfluxDB3 binary..."
cp "$REAL_HOME/Documents/influxdb/target/release/influxdb3" /usr/local/bin/influxdb3
chmod +x /usr/local/bin/influxdb3

echo "==> Patching and installing systemd service files for user '$REAL_USER'..."
for SERVICE in fsae-raspi.service influxdb3.service; do
    sed \
        -e "s|__USER__|$REAL_USER|g" \
        -e "s|__HOME__|$REAL_HOME|g" \
        "$SERVICE" > "/etc/systemd/system/$SERVICE"
done

echo "==> Installing CAN network config..."
cp 80-can.network /etc/systemd/network/

echo "==> Reloading and enabling services..."
systemctl daemon-reload
systemctl enable --now systemd-networkd
systemctl enable --now influxdb3
systemctl enable --now fsae-raspi

echo "==> Done. Services running:"
systemctl is-active influxdb3 fsae-raspi systemd-networkd