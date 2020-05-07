#!/usr/bin/env bash
nasm -o loader.bin loader_nasm.asm 
sudo mount floppy_loader.img ./fat
sudo cp loader.bin fat
# sudo rm fat/kernel.bin
# sudo cp kernel.bin fat
sudo umount fat
bochs -f bochsrc_loader.txt