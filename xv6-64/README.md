# README
[xv6-64](https://github.com/naoki9911/xv6-64) @ `60430e8 on Apr 24, 2019`

xv6-64仅包含bootloader+启动输出调试信息部分, 完整项目请阅读[xv6-64](https://github.com/naoki9911/xv6_uefi)(是32位).

## 构建
```bash
# mkdir -p image/EFI/BOOT
# pushd .
# cd os
# make
# cp kernel ../image
# popd
# pushd .
# cp -rf bootloader <edk2>
# cd <edk2>
# build -p bootloader/xv6-64_loader.dsc
# popd
# cp <edk2>/bootloader/build/DEBUG_GCC5/X64/xv6-64_loader.efi image/EFI/BOOT/bootx64.efi
# qemu-system-x86_64 -machine q35 -bios /usr/share/qemu/OVMF.fd -drive format=raw,file=fat:rw:image -net none -serial stdio
```