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
- 0x00000000fff00000~0x00000000ffffefff -> 0x000000101000-0x0000001fffff : 4095M~4095M+1020k(只占当前页表的后1/3=`254*4k`, 且第768和最后一个页表项已被使用) ~ 1M+4K~1M+1M, create_kernel_pde填充了pde
- 0x00000000fffff000-0x00000000ffffffff -> 0x000000100000-0x000000100fff : 4095M+1020k~4095M+4k ~ 1M ~ 1M+4k, 页表项. 通过虚拟地址 Oxfffffxxx 的方式即可访问页表项, 其中的 xxx是页目录表内的偏移地址

## TLB
TLB(Translation Lookaside Buffer, 俗称快表)是专门用来存放虚拟地址页框与物理地址页框的映射关系.

TLB 中的条目是虚拟地址的高 20 位到物理地址高 20 位的映射结果, 实际上就是从虚拟页框到物理页框的映射. 除此之外 TLB中还有一些属性位, 比如页表项的 RW 属性.

更新TLB:
1. 重载rc3
1. 用指令 invlpg (invalidate page ）刷新某个虚拟地址对应的条目

## kernel
loader.S 需要修改两个地方:
1. 加载内核: 需要把内核文件加载到内存缓冲区
1. 初始化内核: 需要在分页后, 将加载进来的 elf 内核文件安置到相应的虚拟内存地址, 然后跳过去执行, 从此 loader 的工作结束

为了简单, 选择在分页前加载kernel

### 编译
`ld kernel/main.o -Ttext 0xc0001500 -e main -o kernel/kernel.bin`说明:
- `-Ttext`:指定起始虚拟地址0xc0001500
- `-e`: 指定程序的入口符号_start. 使用 entry 作为开始执行程序的显式符号, 而不是默认入口点. 如果没有名为 entry 的符号, 链接器将尝试将 entry 解析为数字

```bash
# gcc -m32 -S -o kernel/main.s kernel/main.c
# cat kernel/main.s
# readelf -a kernel/kernel.bin 
ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Intel 80386
  Version:                           0x1
  Entry point address:               0xc0001500
  Start of program headers:          52 (bytes into file)
  Start of section headers:          8536 (bytes into file)
  Flags:                             0x0
  Size of this header:               52 (bytes)
  Size of program headers:           32 (bytes)
  Number of program headers:         5
  Size of section headers:           40 (bytes)
  Number of section headers:         8
  Section header string table index: 7

Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .text             PROGBITS        c0001500 000500 000014 00  AX  0   0  1
  [ 2] .eh_frame         PROGBITS        c0002000 001000 000048 00   A  0   0  4
  [ 3] .got.plt          PROGBITS        c0004000 002000 00000c 04  WA  0   0  4
  [ 4] .comment          PROGBITS        00000000 00200c 00002b 01  MS  0   0  1
  [ 5] .symtab           SYMTAB          00000000 002038 000090 10      6   4  4
  [ 6] .strtab           STRTAB          00000000 0020c8 000051 00      0   0  1
  [ 7] .shstrtab         STRTAB          00000000 002119 00003d 00      0   0  1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  D (mbind), p (processor specific)

There are no section groups in this file.

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x000000 0xc0001000 0xc0000000 0x000d4 0x000d4 R   0x1000
  LOAD           0x000500 0xc0001500 0xc0001500 0x00014 0x00014 R E 0x1000
  LOAD           0x001000 0xc0002000 0xc0002000 0x00048 0x00048 R   0x1000
  LOAD           0x002000 0xc0004000 0xc0004000 0x0000c 0x0000c RW  0x1000
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RW  0x10

 Section to Segment mapping:
  Segment Sections...
   00     
   01     .text 
   02     .eh_frame 
   03     .got.plt 
   04     

There is no dynamic section in this file.

There are no relocations in this file.
No processor specific unwind information to decode

Symbol table '.symtab' contains 9 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00000000     0 FILE    LOCAL  DEFAULT  ABS main.c
     2: 00000000     0 FILE    LOCAL  DEFAULT  ABS 
     3: c0004000     0 OBJECT  LOCAL  DEFAULT    3 _GLOBAL_OFFSET_TABLE_
     4: c0001510     0 FUNC    GLOBAL HIDDEN     1 __x86.get_pc_thunk.ax
     5: c000400c     0 NOTYPE  GLOBAL DEFAULT    3 __bss_start
     6: c0001500    16 FUNC    GLOBAL DEFAULT    1 main
     7: c000400c     0 NOTYPE  GLOBAL DEFAULT    3 _edata
     8: c000400c     0 NOTYPE  GLOBAL DEFAULT    3 _end

No version information found in this file.
```

程序入口是`Entry point address=0xc0001500`
程序第一个段的VirtAddr是0xc0001000