#!/bin/bash

#rm floppy disk
rm bin/floppy.img

#make floppy image
dd if=/dev/zero of=bin/floppy.img bs=512 count=2880
as86 -0 -b bin/bootsect.bin boot/bootsect.s
as86 -0 -b bin/setup.bin boot/setup.s
dd if=bin/bootsect.bin of=bin/floppy.img conv=notrunc bs=512 seek=0
dd if=bin/setup.bin of=bin/floppy.img conv=notrunc bs=512 seek=1
qemu-system-i386 -m 64M \
    -fda bin/floppy.img\
    -monitor stdio\
    -boot a