# README
from: https://gitee.com/tanyugang/UEFI/tree/main/%E7%AC%AC11%E8%AF%9D

步骤:
1. 先注释部分代码获取FrameBufferBase, 用于生成Kernel.bin
2. 部署
    ```bash
    # cp /home/chen/git/write_os/uefi/env/edk2/Build/MyAppPkg/DEBUG_GCC5/X64/Bootloader.efi ../root2
    # cp Bootloader-Simple/Kernel/Kernel.bin ../root2
    # cp Bootloader-Simple/Logo.BMP ../root2
    # cp Bootloader-Simple/Narrow.BMP ../root2
    # qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -drive format=raw,file=fat:rw:root2 -net none
    ```

在uefi shell 执行Bootloader.efi, 效果: 全屏蓝色, 未成功???