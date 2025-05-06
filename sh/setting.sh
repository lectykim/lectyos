#!/bin/bash

#rm floppy disk
rm bin/floppy.img

#make floppy image
dd if=/dev/zero of=bin/floppy.img bs=512 count=2880
as86 -b bin/bootsect.bin bin/bootsect.s
dd if=bin/bootsect.bin of=bin/floppy.img conv=notrunc bs=512 seek=0
qemu-system-i386 -m 16M -fda bin/floppy.img -boot a