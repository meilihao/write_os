# README

查看页表:
```bash
# bochs -f bochsrc
<bochs:1> c
^CNext at t=32157761
(0) [0x000000000a27] 0008:0000000000000a27 (unk. ctxt): jmp .-2  (0x00000a27)     ; ebfe
<bochs:2> info tab
cr3: 0x000000100000
0x0000000000000000-0x00000000000fffff -> 0x000000000000-0x0000000fffff
0x00000000c0000000-0x00000000c00fffff -> 0x000000000000-0x0000000fffff
0x00000000ffc00000-0x00000000ffc00fff -> 0x000000101000-0x000000101fff
0x00000000fff00000-0x00000000ffffefff -> 0x000000101000-0x0000001fffff
0x00000000fffff000-0x00000000ffffffff -> 0x000000100000-0x000000100fff
```

说明:
- 0x0000000000000000-0x00000000000fffff: 第0个页表, 指向低端1M内存
- 0x00000000c0000000-0x00000000c00fffff: 第768个页表, 指向低端1M内存

其他三行映射有点怪异, 原因是`mov [PAGE_DIR_TABLE_POS + 4092], eax`导致, 此时是将页目录项当做一个页表来使用, 叫页目录自映射:
- 0x00000000ffc00000-0x00000000ffc00fff -> 0x000000101000-0x000000101fff : 4092M~4092M+4k ~ 1M+4K~1M+8K, 第一个页表
- 0x00000000fff00000~0x00000000ffffefff -> 0x000000101000-0x0000001fffff : 4095M~4095M+1020k ~ 1M+4K~1M+1M, create_kernel_pde填充了pde
- 0x00000000fffff000-0x00000000ffffffff -> 0x000000100000-0x000000100fff : 4095M+1020k~4095M+4k ~ 1M ~ 1M+4k, 页表项. 通过虚拟地址 Oxfffffxxx 的方式即可访问页表项, 其中的 xxx是页目录表内的偏移地址

## TLB
TLB(Translation Lookaside Buffer, 俗称快表)是专门用来存放虚拟地址页框与物理地址页框的映射关系.

TLB 中的条目是虚拟地址的高 20 位到物理地址高 20 位的映射结果, 实际上就是从虚拟页框到物理页框的映射. 除此之外 TLB中还有一些属性位, 比如页表项的 RW 属性.

更新TLB:
1. 重载rc3
1. 用指令 invlpg (invalidate page ）刷新某个虚拟地址对应的条目

## kernel
`ld kernel/main.o -Ttext 0xc0001500 -e main -o kernel/kernel.bin`说明:
- `-Ttext`:指定起始虚拟地址0xc0001500
- `-e`: 指定程序的入口符号_start. 使用 entry 作为开始执行程序的显式符号, 而不是默认入口点. 如果没有名为 entry 的符号, 链接器将尝试将 entry 解析为数字