#!/usr/bin/env bash
#modprobe nbd max_part=8
qemu-nbd --connect=/dev/nbd0 os.qcow2
mkdir -p root
mount /dev/nbd0p2 root # as /
mkdir -p root/boot
mount /dev/nbd0p1 root/boot # as /boot
# cp ../Cosmos/initldr/build/Cosmos.eki root/boot # 见../README.md FAQ的`INITKRL DIE ERROR：not find file`
cp ../../lesson13/Cosmos/release/Cosmos.eki root/boot
rm -rf root/boot/HelloOS.bin
rm -rf root/boot/grub/grub.cfg
cat > root/boot/grub/grub.cfg << "EOF"
set default=0
set timeout=5

menuentry 'HelloOS' {
     insmod part_msdos # GRUB加载分区模块识别分区
     insmod ext2 # GRUB加载ext文件系统模块识别ext文件系统
     set root='hd0,msdos1' # 注意boot目录挂载的分区, hd0,msdos1表示第一块hd的第一个分区, grub标记分区从1开始
     multiboot2 /Cosmos.eki
     boot
}
# [Adding a grub menu option to reboot to the BIOS / UEFI settings on CentOS](https://jsherz.com/centos/grub/grub2/bios/uefi/boot/2015/11/21/centos-uefi-firmware-option.html), not work, fwsetup is for uefi 且使用前需要insmod efifwsetup
menuentry 'Enter setup' {
     fwsetup
}
EOF
umount root/boot
umount root
qemu-nbd --disconnect /dev/nbd0
