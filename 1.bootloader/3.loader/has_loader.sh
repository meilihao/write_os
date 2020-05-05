#!/usr/bin/env bash
as  -o loader.o loader.s
ld -Ttext 0x0 --oformat=binary loader.o -o loader.bin
sudo mount floppy_loader.img ./fat
sudo cp loader.bin fat
sudo umount fat
# bochs -f bochsrc_loader.txt