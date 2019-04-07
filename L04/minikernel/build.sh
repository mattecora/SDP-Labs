#!/bin/bash

# go to kernel dir
cd kernel

# build kernel
make

# disassemble kernel
objdump -d kernel > kernel.asm.txt

cd ..