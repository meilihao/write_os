# 准备工作环境

from https://gitee.com/xiyue87/efi-and-bochs-loader-for-x64 for 学习bootloader

原始参考：

https://zhuanlan.zhihu.com/p/491111229

https://zhuanlan.zhihu.com/p/491813120

## 软件

1. NASM 
2. Visual Studio 2019
3. Virtual Box
4. Bochs

## 运行库

1. EFI_Toolkit

# 编译

## 编译BochsLoader

### 编译代码

编辑BochsLoader下的makefile文件，指定nasm路径

启动x86 Native Tools Command Prompt for VS 2019，进入BochsLoader，运行nmake即可编译。

### 准备环境

编译完成以后，使用objs\mkbtfat32.exe将FAT32LDR.bin写入虚拟机C盘，注意要设置好活动分区，并且分区是FAT32类型。

把objs\SYSLDR.BIN复制虚拟机C盘根目录下，启动bochs模拟器。目录下的bochsrc-sysldr.bxrc可以作为模拟器的配置参考。模拟器需要配置硬盘启动+vhd镜像+串口，内存需要稍微大一点。

## 编译EFILoader

事先编译好EFI_Toolkit，修改EFILoader下的Makefile里的INCLUDE指向，到EFI_Toolkit

启动x64 Native Tools Command Prompt for VS 2019，进入EFILoader，运行nmake即可编译。

复制objs\EFILoader.exe到虚拟机C盘的\EFI\BOOT\BOOTX64.EFI

## 编译64位镜像

启动x64 Native Tools Command Prompt for VS 2019，进入Krnl64，运行nmake即可编译。

将objs\HELLO64.EXE复制到虚拟机C盘根目录下，启动bochs模拟器或者VirtualBox即可。

串口输出Hello long mode说明加载成功

