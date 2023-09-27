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

### kernelmemfs LOAD PhysAddr=0x8010acf6
```
# readelf -l kernelmemfs

Elf file type is EXEC (Executable file)
Entry point 0x100010
There are 3 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x80100000 0x00100000 0x0acf6 0x0acf6 R E 0x1000
  LOAD           0x00bcf6 0x8010acf6 0x8010acf6 0x7f84c 0x8a84a RW  0x1000
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x10

 Section to Segment mapping:
  Segment Sections...
   00     .text .rodata
   01     .stab .stabstr .data .bss
   02
# readelf -S kernelmemfs
There are 19 section headers, starting at offset 0xb8f5c:

Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .text             PROGBITS        80100000 001000 008a23 00  AX  0   0 16
  [ 2] .rodata           PROGBITS        80108a40 009a40 0022b6 00   A  0   0 32
  [ 3] .stab             PROGBITS        8010acf6 00bcf6 000001 0c  WA  4   0  1
  [ 4] .stabstr          STRTAB          8010acf7 00bcf7 000001 00  WA  0   0  1
  [ 5] .data             PROGBITS        8010b000 00c000 07f542 00  WA  0   0 4096
  [ 6] .bss              NOBITS          8018a560 08b542 00afe0 00  WA  0   0 32
....
```

`LOAD PhysAddr=0x8010acf6`导致bootloader RelocateELF报错

根据`Section to Segment mapping`, `01 LOAD`是`.stab .stabstr .data .bss`, 结合`Section header table`可获得各session的布局.

用了xv6_uefi's xv6_public fork的0754d21c865e97582968fa5d155eac133e5829b0版, 该LOAD的PhysAddr是0x8010798e, 但`mit-pdos/xv6-public@eeb7b41`是0x00108000.

推测问题处在ld时, 对比xv6-public 0754d21c和eeb7b41的kernel.ld, 找到差异[Remove BYTE directives from kernel linker script to fix triple fault on boot](https://github.com/mit-pdos/xv6-public/commit/1db17ac1fdb70cd98dfc49d50e89f8abcff9a092), 修正kernel.ld后, 该LOAD的PhysAddr是0x0010b000.


