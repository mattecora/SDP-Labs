#!/bin/bash

if [ $# -ne 1 ]
then
echo Usage: $0 number of k-bytes
exit 1
fi

kbytes=$1

# save these to chmod files back
var_user=matteo
var_group=matteo

# create an empty disk image
dd if=/dev/zero of=hd.img bs=1 count=$((kbytes*1024))

# prepare fdisk input
cat <<EOF >> fdisk.input
n
p
1


a
w
EOF

# run fdisk to create a partition
fdisk hd.img < fdisk.input

# map the image to the loop0 device, leaving the first 512 bytes out
losetup -o 512 /dev/loop0 hd.img

# create a file system on loop0
mkfs.ext2 /dev/loop0

# mount the image
mount /dev/loop0 /mnt

# create menu.lst for grub
cat <<EOF > menu.lst
title Minimal Linux-like kernel
kernel /boot/kernel root=/dev/hda ro quiet splash
quiet
EOF

# copy files from local hard drive
mkdir -p /mnt/boot/grub
cp grub/stage1 grub/stage2 menu.lst /mnt/boot/grub
cp kernel/kernel /mnt/boot/kernel

# umount the image
umount /mnt

# detach loop0 
losetup -d /dev/loop0

# create input file for grub
cat <<EOF > grub.input
device (hd0,0) hd.img
root (hd0,0)
setup (hd0)
quit
EOF

# install grub
grub/grub.bin --device-map=/dev/null < grub.input

# transfer the ownership
chown ${var_user}:${var_group} hd.img

# remove temp files
rm fdisk.input
rm grub.input
rm menu.lst