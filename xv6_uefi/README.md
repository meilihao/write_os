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
1. 用`xv6_public/tree/uefi@latest`替代`xv6_public @ 633b564`(`make kernelmemfs`报错)

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
