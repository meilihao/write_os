# README
ref:
- [How to start from QEMU with GRUB](https://github.com/xymeng16/helloOS/blob/main/docs/using-qemu.md)

[HelloOS](https://gitee.com/lmos/cosmos/blob/master/lesson02/HelloOS)

> 其实可以删除grub 1的引导头, 当前主流是v2

```bash
qemu-img create -f qcow2 os.qcow2 64M # 制作raw image, 则可用losetup代替nbd
modprobe nbd max_part=8
qemu-nbd --connect=/dev/nbd0 os.qcow2
./fdisk.sh # 当前使用了构建`/`和`/boot`的两分区方案. 其实直接使用一个`/`分区也可以, 这里改为: `printf "o\nn\np\n1\n\n\nw\n" | fdisk helloOS.img`
mkdir root
mount root /dev/nbd0p2 # as /
mkdir root/boot
mount /dev/nbd0p1 root/boot # as /boot
grub-install --boot-directory=root/boot --target=i386-pc --recheck /dev/nbd0
cp HelloOS.bin root/boot
cat > root/boot/grub/grub.cfg << "EOF"
set default=0
set timeout=5

menuentry 'HelloOS' {
     insmod part_msdos # GRUB加载分区模块识别分区
     insmod ext2 # GRUB加载ext文件系统模块识别ext文件系统
     set root='hd0,msdos1' # 注意boot目录挂载的分区, hd0,msdos1表示第一块hd的第一个分区, grub标记分区从1开始
     multiboot2 /HelloOS.bin # GRUB以multiboot2协议加载HelloOS.bin. 这里路径不能用`/boot/HelloOS.bin`见`file '/boot/HelloOS.bin' not found`
     boot # GRUB启动HelloOS.bin
}
EOF
umount root/boot
umount root
qemu-nbd --disconnect /dev/nbd0
qemu-system-i386 -hda os.qcow2
```


其他参考:
- [不需grub](https://github.com/vizv/learn_os/blob/master/hello-os)


## FAQ
### grub 选中`HelloOS`后启动报`file '/boot/HelloOS.bin' not found`
默认载入时就已经以/boot为根目录, 没有额外设定/boot目录

解决方法: `multiboot2 /boot/HelloOS.bin`改为`multiboot2 /HelloOS.bin`