#!/bin/bash

# Install the module
insmod chardev_SDP_lab.ko

# Show the log
echo "Module installed. Log:"
dmesg | tail -n 4

# Get the command to run
mknod=$(dmesg | tail -n 4 | grep mknod -m 1 | cut -d "'" -f 2)

# Create the device
eval $mknod