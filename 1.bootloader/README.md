# bootloader
参考:
- [嵌入式系统 Boot Loader 技术内幕](https://www.ibm.com/developerworks/cn/linux/l-btloader/index.html)

bootloader拆成boot(stage1), loader(stage1.5,识别相应的文件系统 + stage2)两部分的原因:
mbr内容有限, loader对空间的需求日益增长.

loader, 常见的有grub2