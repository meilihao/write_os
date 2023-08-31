#!/usr/bin/env bash
qemu-nbd --connect=/dev/nbd0 os.qcow2
mkdir -p root
mount /dev/nbd0p2 root # as /
mkdir -p root/boot
mount /dev/nbd0p1 root/boot # as /boot
cp ../Cosmos/initldr/build/Cosmos.eki root/boot
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
EOF
umount root/boot
umount root
qemu-nbd --disconnect /dev/nbd0