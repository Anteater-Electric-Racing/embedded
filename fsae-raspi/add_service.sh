#!/bin/bash

SERVICE_FILE="fsae-raspi.service"
DEST_DIR="/etc/systemd/system/"

if [ "$(id -u)" -ne 0 ]; then
    echo "Please run this script as root (e.g., using sudo)." >&2
    exit 1
fi

echo "Copying ${SERVICE_FILE} to ${DEST_DIR}..."
cp "$SERVICE_FILE" "$DEST_DIR" || { echo "Failed to copy service file."; exit 1; }

echo "Reloading systemd daemon..."
systemctl daemon-reload

echo "Enabling the service..."
systemctl enable fsae-raspi.service

echo "Starting the service..."
systemctl start fsae-raspi.service

echo "Service fsae-raspi is now enabled and running."
