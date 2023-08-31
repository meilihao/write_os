# README
二级引导器作为操作系统的先驱，它需要收集机器信息，确定这个计算机能不能运行操作系统，对 CPU、内存、显卡进行一些初级的配置，放置好内核相关的文件.

[二级引导器功能划分表](misc/3169e9db4549ab036c2de269788a281e.webp)

GRUB 头有两个文件组成:
1. imginithead.asm

	它有两个功能，既能让 GRUB 识别，又能设置 C 语言运行环境，用于调用 C 函数.

	它主要工作是初始化 CPU 的寄存器，加载 GDT，切换到 CPU 的保护模式.
1. inithead.c

	它的主要功能是查找二级引导器的核心文件——initldrkrl.bin，然后把它放置到特定的内存地址上

## 构建img
构建img工具[lmoskrlimg](../../../tools/lmoskrlimg/)