#!/usr/bin/env bash
as  -o boot.o boot.s
ld -Ttext 0x7c00 --oformat=binary boot.o -o boot.bin
dd if=boot.bin of=floppy_loader.img  bs=512 count=1 conv=notrunc
# bochs -f bochsrc_loader.txt