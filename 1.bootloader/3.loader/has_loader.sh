#!/usr/bin/env bash
# floppy_loader.img : copy from 2.loader's floppy_loader.img (include boot)
as  -o loader.o loader.s
ld -Ttext 0x0 --oformat=binary loader.o -o loader.bin
sudo mount floppy_loader.img ./fat
sudo cp loader.bin fat
sudo cp kernel.bin fat # kernel.bin必须与内容
sudo umount fat
# bochs -f bochsrc_loader.txt