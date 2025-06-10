#!/bin/bash

NETWORK_FILE="80-can.network"
SRC_DIR="$(pwd)"
DEST_DIR="/etc/systemd/network/"

if [ "$(id -u)" -ne 0 ]; then
    echo "Please run this script as root (e.g., using sudo)." >&2
    exit 1
fi

if [ -f "${DEST_DIR}${NETWORK_FILE}" ]; then
    echo "Removing existing network file..."
    rm "${DEST_DIR}${NETWORK_FILE}"
fi

echo "Copying ${NETWORK_FILE} to ${DEST_DIR}..."
cp "${SRC_DIR}/${NETWORK_FILE}" "${DEST_DIR}" || { echo "Failed to copy network file."; exit 1; }

echo "Enabling systemd-networkd service..."
systemctl enable systemd-networkd

echo "Starting systemd-networkd service..."
systemctl start systemd-networkd

echo "Network configuration applied and systemd-networkd is running."