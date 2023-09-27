# xv6_uefi @ 8fdda2b on Sep 13, 2018
xv6_uefi is the operating system based on xv6.
This operating system runs on x86-64 and uefi system.

```
git clone https://github.com/naoki9911/xv6_uefi
cd xv6_uefi
make
```
If you have some trouble, try to execute commands line by line in build.sh
# Packages
Needed packages are as follows.

```
qemu
nasm
acpica
```

# TODO
- console with frame buffer - Done
- nic device driver - Almost Done
- tcp/ip protocol stack - In Progress
- HTTP server

## 部署
改动:
1. 将edk2 @ f6392fd的xv6_bootloader迁入项目
1. 修复`xv6_public @ 633b564`和`xv6_public/tree/uefi@latest=57d193e`执行`make kernelmemfs`报错, 见FAQ


```
# pushd .
# cp -r xv6_bootloader <edk2>
# cd <edk2>
# build -p xv6_bootloader/xv6_bootloader.dsc -a X64 # xv6_bootloader是64位的, 比如使用了`lretq`
# cp /home/chen/git/write_os/uefi/env/edk2/xv6_bootloader/build/DEBUG_GCC5/X64/loader.efi image/EFI/BOOT/bootx64.efi
# popd
# cp xv6_bootloader/logo.bmp image/
# pushd .
# cd xv6_public
# make kernelmemfs
# cp kernelmemfs ../image/kernel
# popd
# qemu-system-x86_64 -machine q35 -bios /usr/share/qemu/OVMF.fd -drive format=raw,file=fat:rw:image -net none -serial stdio
```

实测: 显示logo时花屏并重启

## FAQ
### `make kernelmemfs`报`ld: vm.o:.../xv6_public-633b564fadae93e054fe038cba5febc77ce160f0/graphic.h:34: multiple definition of `gpu'; console.o:.../xv6_public-633b564fadae93e054fe038cba5febc77ce160f0/graphic.h:34: first defined here`
ref:
- [multiple definition of `xxxx`问题解决及其原理](https://blog.csdn.net/mantis_1984/article/details/53571758)

解决方法: 在.c程序中定义全局变量，在.h文件中使用extern 做外部声明，供其他文件调用, 目的其实只有一个，就是使变量在内存中唯一化.

graphic.c
```c
#include "defs.h"

struct gpu gpu;

/*
 * i%4 = 0 : blue
 * i%4 = 1 : green
 * i%4 = 2 : red
 * i%4 = 3 : black
 */
void graphic_init(){
```

graphic.h
```c
void graphic_scroll_up(int height);

extern struct gpu gpu;

#endif
```



