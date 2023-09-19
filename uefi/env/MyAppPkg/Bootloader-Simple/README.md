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

在uefi shell 执行Bootloader.efi, 效果: 全屏蓝色, 未成功, 但在GetMemoryMap后加Print就能成功???

## 改进
- [SimpleUefiLog](https://www.jianshu.com/p/9ae9c24d08ec)

## FAQ
### bmp
```
# convert 16.webp -alpha set -define bmp:format=bmp3 -define bmp3:alpha=true test.bmp # bits offset 54
# file test.bmp 
test.bmp: PC bitmap, Windows 3.x format, 240 x 240 x 32, image size 230400, cbSize 230454, bits offset 54
```

注意图片格式, 有的bmp图片格式有其他处理. 当前项目需使用**image size=cbSize-54**的bmp图片, 推荐使用上述命令生成bmp.