#!/bin/bash

echo "Writing $1 to chardev..."
echo $1 > /dev/chardev_SDP_lab

echo "Reading "$1" from chardev..."
cat /dev/chardev_SDP_lab # Should print the first argument