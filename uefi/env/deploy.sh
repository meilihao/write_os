#!/usr/bin/env bash
qemu-nbd --connect=/dev/nbd0 hda.img

mkdir -p root
mount /dev/nbd0p3 root # as /
mkdir -p root/boot
mount /dev/nbd0p2 root/boot # as /boot
mkdir -p root/boot/efi
mount /dev/nbd0p1 root/boot/efi # as /boot/efi

# ---
# 放置设置代码
cp /usr/lib/grub/x86_64-efi/monolithic/grubx64.efi root/boot/efi/bootx64.efi # for demo, 在uefi shell执行bootx64.efi可进入grub shell
sync
# ---

umount root/boot/efi
umount root/boot
umount root

qemu-nbd --disconnect /dev/nbd0