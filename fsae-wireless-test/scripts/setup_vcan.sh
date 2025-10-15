#!/bin/bash
# create a VCAN for testing 

sudo modprobe vcan

if ip link show vcan0 &> /dev/null; then
    echo "vcan0 already exists"
else
    echo "creating vcan0"
    sudo ip link add dev vcan0 type vcan
fi 

sudo ip link set vcan0 up
echo "vcan is running"