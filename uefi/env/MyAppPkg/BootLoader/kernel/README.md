# README
```bash
cd kernel
make all
```

## FAQ:
### `make all`报`relocation truncated to fit: R_X86_64_PLT32 against undefined symbol `__switch_to'`
ref:
- [Relocation overflow and code models](https://zhuanlan.zhihu.com/p/629373116)

[OS-Virtual-Platform/kernel/Makefile](https://gitee.com/MINEOS_admin/OS-Virtual-Platform/blob/develop/kernel/Makefile)

使用了`-std=gnu89`, 该报错消失