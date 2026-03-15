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

echo "==> Building TDEngine..."
TD_VER="3.4.0.9"
wget -q "https://downloads.tdengine.com/tdengine-tsdb-oss/${TD_VER}/tdengine-tsdb-oss-${TD_VER}-linux-${ARCH}.tar.gz"
tar -zxf "tdengine-tsdb-oss-${TD_VER}-linux-${ARCH}.tar.gz"
cd "tdengine-tsdb-oss-${TD_VER}" && ./install.sh && cd -
rm -rf "tdengine-tsdb-oss-${TD_VER}"*
systemctl enable --now taosd

echo "==> Patching and installing systemd service files for user '$REAL_USER'..."
for SERVICE in fsae-raspi.service; do
    sed \
        -e "s|__USER__|$REAL_USER|g" \
        -e "s|__HOME__|$REAL_HOME|g" \
        "$SERVICE" > "/etc/systemd/system/$SERVICE"
    systemctl daemon-reload
    systemctl enable --now "$SERVICE"
done

echo "==> Installing CAN network config..."
cp 80-can.network /etc/systemd/network/