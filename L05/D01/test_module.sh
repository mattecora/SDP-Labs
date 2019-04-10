#!/bin/bash

# Install the module
insmod hello-5.ko

echo "Module installed. Log:"
echo ""

# Show the kernel log
dmesg | tail

# Uninstall the module
rmmod hello-5.ko

echo ""
echo "Module uninstalled. Log:"

# Show the kernel log
dmesg | tail