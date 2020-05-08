#!/usr/bin/env bash
# floppy_loader.img : copy from 1.boot's floppy.img
as  -o boot.o boot.s
ld -Ttext 0x7c00 --oformat=binary boot.o -o boot.bin
dd if=boot.bin of=floppy_loader.img  bs=512 count=1 conv=notrunc

as  -o loader.o loader.s
ld -Ttext 0x0 --oformat=binary loader.o -o loader.bin
sudo mount floppy_loader.img ./fat
sudo cp loader.bin fat
sudo umount fat
bochs -f bochsrc_loader.txt