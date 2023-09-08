#!/usr/bin/env bash
qemu-nbd --connect=/dev/nbd0 hda.img
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | gdisk /dev/nbd0
  o # new gpt
  Y # Proceed
  n
  1

  +16M
  ef00 # EFI System
  n
  2

  +16M
  8300
  n
  3
  
  
  
  w # write GPT data
  Y # want to proceed
EOF

gdisk -l /dev/nbd0
fdisk -l /dev/nbd0

mkfs -t vfat /dev/nbd0p1
mkfs -t ext4 /dev/nbd0p2
mkfs -t ext4 /dev/nbd0p3

qemu-nbd --disconnect /dev/nbd0