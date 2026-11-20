# write os
参照<<一个64位操作系统的设计与实现>>写一个64 bit的os.

目的:
1. 理解os
1. just for fun!

模拟器选择qemu, 兼容性更好, 更容易发现问题, 调试可针对场景bochs/qemu一起上.

## books
建议仅关注UEFI + 64位操作系统.

- <<一个64位操作系统的设计与实现 第二版>>
	- [code v2](https://www.ituring.com.cn/book/3488)
	- [~~code v1~~](https://github.com/yifengyou/The-design-and-implementation-of-a-64-bit-os)
	- [next code](https://gitee.com/MINEOS_admin/publish)

		- [publish/kernel/DeviceDriver](https://gitee.com/MINEOS_admin/publish/tree/develop/kernel/DeviceDriver)有很多驱动, 比如AHCI
	- [author + new](https://www.ituring.com.cn/space/186920)
	- [**sOS64**](https://github.com/jiebaomaster/sOS64)

		支持uefi+bios

		- [64bitOS](https://github.com/JiaweiHH/64bitOS)

- [**现代 64 位操作系统开发**](https://zhuanlan.zhihu.com/p/669481626)

	[原文](https://arttnba3.cn/2023/11/29/CODE-0X03-OSDEV64-I_UEFI-GRUB/)
-  ~~操作系统真象还原~~
	- [《操作系统真象还原》源码及学习笔记（os-elephant）还原真相](https://github.com/yifengyou/os-elephant)
	- [操作系统真象还原 学习笔记19--总结&感悟](https://www.kn0sky.com/?p=60)

	2016年出版, 32位的操作系统

- 从零自制操作系统

	- [mikanos-build](https://github.com/uchan-nos/mikanos-build/tree/master)
- 穿越操作系统迷雾
- ~~30天自制操作系统~~

	2006年出版, 距今硬件已大变
- ~~Orange's 一个操作系统的实现~~

	2009年出版, 32位的操作系统
- x86/x64体系探索及编程
- [从零开始写 OS 内核](https://segmentfault.com/a/1190000040124650)
- [xbook2](https://github.com/hzcx998/xbook2)
- **xv6**

	建议使用[master on eeb7b41@2020-08-10], 比xv6-rev11有少量改动.

	- [xv6-labs](https://pdos.csail.mit.edu/6.828/2025/schedule.html) + [xv6-labs-2022-solutions](https://github.com/relaxcn/xv6-labs-2022-solutions/tree/main)
- [跟我一起写操作系统](https://www.askpure.com/course_KH3775IW-COUS3DX1-1Y2VUM3Q-CE78MA15.html)

## ref
- [x86汇编语言](https://www.jianshu.com/p/7f63e62e0ffd)

## repo
- [GrapeOS操作系统](https://gitee.com/jackchengyujia/grapeos-course)
- [toaruos](https://github.com/klange/toaruos)

	- [12 Years of ToaruOS](https://gist.github.com/klange/f427a551af5f2f8b3c9ef80687883fcf)
- [Google KataOS]()
- [自制操作系统日记](https://developer.aliyun.com/article/1080227)
- [RVOS的石榴派适配](https://gitee.com/RV4Kids/PLCT-Weekly/issues/I43DNU)

	[开发一个RISC-V上的操作系统](https://gitee.com/RV4Kids/RVWeekly/issues/I3I74V)

- [这才是计科之 Onix & XV6 源码分析（1、XV6-x86的启动）](https://blog.csdn.net/m0_52566365/article/details/139523887)
- [zCore](https://github.com/rcore-os/zcore)
- [rCore](https://github.com/rcore-os/rcore)

	- [**rCore-Tutorial-Book-v3**](https://github.com/rcore-os/rCore-Tutorial-Book-v3)

	已被zCore取代
- [StarryOS](https://rcore-os.cn/blog/2026/04/25/%E5%AE%9E%E4%B9%A0%E6%8A%80%E6%9C%AF%E6%8A%A5%E5%91%8A%EF%BC%9AStarryOS-%E5%86%85%E6%A0%B8%E6%A8%A1%E5%9D%97%E6%8C%89%E9%9C%80%E5%8A%A0%E8%BD%BD%E4%B8%8E-FUSE-%E7%94%A8%E6%88%B7%E6%80%81%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F%E9%A9%B1%E5%8A%A8%E5%AE%9E%E7%8E%B0/)


todo:
- [Luos](https://mp.weixin.qq.com/s/L0eZCsIXMIfeMKZO190XWg)

## uefi
- [UEFI BootLoader for ToyOS.](https://github.com/tanyugang/UEFI)
- [uefi_specs_personal_collections](https://github.com/xiaopangzi313/uefi_specs_personal_collections)
- [openEuler / lboot](https://gitee.com/openeuler/lboot)

	由 Rust 语言实现的轻量级引导加载程序
- [0x06 从零开始开发操作系统 UEFI](https://zhuanlan.zhihu.com/p/683894156)
- [从零开始的UEFI裸机编程](https://kagurazakakotori.github.io/ubmp-cn/index.html)