# README

## build
```bash
cd edk2
build -p OvmfPkg/OvmfPkgX64.dsc -a X64 -D DEBUG_ON_SERIAL_PORT # ??? 自己编译的ovmf固件无法启动QEMU emulator version 6.2.0 (Debian 1:6.2+dfsg-2ubuntu6.13) + edk2-stable202308, 串口日志报`Broken CPU hotplug register block found`. 使用qemu自带的ovmf固件正常
cp Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd ..
cd MyAppPkg
build -p MyAppPkg/MyAppPkg.dsc -a X64 # `-p`是以edk2目录为根
```

## example
- [一个UEFI引导程序的实现](https://www.ituring.com.cn/book/2763)

    - Memory
    - Video
    - VideoSet
    - File

## FAQ
### File/directory not found in workspace
没有指定env PACKAGES_PATH, 获取其包含相对路径

### `Module for [X64] is not a component of active platform`
1. dsc中的component块中没有指定inf文件的路径导致编译的时候dsc文件和inf文件不匹配，这个时候要往dsc文件中的component块中添加inf文件相对于udk 根目录的路径即可
2. build时没指定`-p xxx.dsc`

### `Instance of library class [RegisterFilterLib] is not found`
在edk2中搜索`RegisterFilterLib`, 找到后加入dsc的`LibraryClasses`

### 自编译ovmf启动卡住
ref:
- [bf5678b OvmfPkg/PlatformInitLib: catch QEMU's CPU hotplug reg block regression](https://pagure.io/lersek/edk2/c/bf5678b5802685e07583e3c7ec56d883cbdd5da3?branch=master)

根据串口输出, 追加`-fw_cfg name=opt/org.tianocore/X-Cpuhp-Bugcheck-Override,string=yes`即可不卡住

ps:
自编译ovmf时, 日志里有多处`...Offset.raw] Error 1 (ignored)`, 比如`make: [GNUmakefile:500: /home/chen/git/write_os/uefi/env/edk2/Build/OvmfX64/DEBUG_GCC5/FV/Ffs/86CDDF93-4872-4597-8AF9-A35AE4D3725FIScsiDxe/IScsiDxeOffset.raw] Error 1 (ignored)`, 文件IScsiDxeOffset.raw并不存在, 原因未知, 不影响ovmf启动