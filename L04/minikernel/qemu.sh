qemu-system-i386 -hda $1 -serial mon:stdio -S -gdb tcp::1234 &
ddd