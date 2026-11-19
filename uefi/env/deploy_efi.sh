#!/usr/bin/env bash

set -ex

if [ $# -gt 0 ]
then
	EFI_FILE=$1
    echo "using <efi path>: $EFI_FILE"
	
	# 没有`-f raw`, qemu-nbd --connect后会直接退出???
	qemu-nbd -f raw --connect=/dev/nbd0 hda.img
	udevadm settle # 等待`qemu-nbd --connect`完成
	blockdev --getsize64 /dev/nbd0

	mkdir -p root
	mount /dev/nbd0p3 root # as /
	mkdir -p root/boot
	mount /dev/nbd0p2 root/boot # as /boot
	mkdir -p root/boot/efi
	mount /dev/nbd0p1 root/boot/efi # as /boot/efi
	mkdir -p root/boot/efi/EFI/BOOT

	# ---
	# 放置设置代码
	# rm -rf root/boot/efi/EFI/BOOT/BOOTX64.EFI
	\cp $EFI_FILE root/boot/efi # root/boot/efi/EFI/BOOT/BOOTX64.EFI # for demo, 在uefi shell执行bootx64.efi可进入grub shell. grub shell: /usr/lib/grub/x86_64-efi/monolithic/grubx64.efi
	sync
	# ---

	sleep 1 # 避免报umount: root/boot/efi: target is busy

	umount root/boot/efi
	umount root/boot
	umount root

	qemu-nbd --disconnect /dev/nbd0

	sleep 3 # 避免qemu-system-x86_64启动并接管终端后, 终端被清屏
fi
