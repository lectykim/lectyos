#!/bin/bash

#rm floppy disk
rm bin/floppy.img

#make floppy image
dd if=/dev/zero of=bin/floppy.img bs=512 count=2880
as86 -b bin/bootsect.bin boot/bootsect.s
as86 -b bin/setup.bin boot/setup.s
as --32 -o bin/head.o boot/head.s
gcc -m32 -ffreestanding -fno-builtin -fno-pic -fno-pie -c init/main.c -o bin/main.o
ld -m elf_i386 -Ttext=0x0 --oformat=binary bin/head.o bin/main.o -o bin/kernel.bin
dd if=bin/bootsect.bin of=bin/floppy.img conv=notrunc bs=512 seek=0
dd if=bin/setup.bin of=bin/floppy.img conv=notrunc bs=512 seek=1
dd if=bin/kernel.bin of=bin/floppy.img conv=notrunc bs=512 seek=5
qemu-system-i386 -m 64M \
    -drive file=bin/floppy.img,if=floppy,format=raw\
    -monitor stdio\
    -boot a
    