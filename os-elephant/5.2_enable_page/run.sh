#!/bin/bash
#bximage -func=create -hd=16M -imgmode="flat" -sectsize=512 -q hd.img # size<10会报`Hard disk image size out of range`
#nasm -I include/ -o mbr.bin mbr.s && dd if=mbr.bin of=./hd.img bs=512 count=1  conv=notrunc
nasm -I include/ -o loader.bin loader.s && dd if=loader.bin of=./hd.img bs=512 count=4 seek=2 conv=notrunc
qemu-system-i386 -hda hd.img