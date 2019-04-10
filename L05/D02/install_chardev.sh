#!/bin/bash

# Install the module
insmod chardev_SDP_lab.ko

# Show the log
echo "Module installed. Log:"
dmesg | tail

# Get the major number
major=$(dmesg | tail -n 4 | grep mknod -m 1 | cut -d ' ' -f 6)

# Create the device
mknod /dev/chardev_SDP_lab c $major 0