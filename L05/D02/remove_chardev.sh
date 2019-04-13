#!/bin/bash

# Remove the device
rm /dev/chardev_SDP_lab

# Uninstall the module
rmmod chardev_SDP_lab.ko

# Show the log
echo "Module removed. Log:"
dmesg | tail -n 1