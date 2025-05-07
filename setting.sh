#!/bin/bash

#rm floppy disk
rm bin/floppy.img

#make floppy image
dd if=/dev/zero of=bin/floppy.img bs=512 count=2880
as86 -b bin/bootsect.bin boot/bootsect.s
as86 -b bin/setup.bin boot/setup.s
as86 -b bin/head.bin boot/head.s
dd if=bin/bootsect.bin of=bin/floppy.img conv=notrunc bs=512 seek=0
dd if=bin/setup.bin of=bin/floppy.img conv=notrunc bs=512 seek=1
dd if=bin.head.bin of=bin/floppy.img conv=notrunc bs=512 seek=$((1+4))
qemu-system-i386 -m 64M \
    -drive file=bin/floppy.img,if=floppy,format=raw\
    -monitor stdio\
    -boot a