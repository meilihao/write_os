# list
## ref
- [Sending data from UEFI to OS through UEFI variables](https://davysouza.medium.com/sending-data-from-uefi-to-os-through-uefi-variables-b4f9964e1883)

## uefi shell env:
ref:
- [X86 CPU的工作模式](https://zhuanlan.zhihu.com/p/298033676)
- [Table 5-1. Supported Paging Alternatives (CR0.PG=1)](https://www.amd.com/content/dam/amd/en/documents/processor-tech-docs/programmer-references/24593.pdf)
- [【UEFI实战】x86平台UEFI下的内存管理](https://blog.csdn.net/jiangwei0512/article/details/64204267)

	读取GDTR可用`AsmReadGdtr()`
- [【x86架构】内存管理](https://blog.csdn.net/jiangwei0512/article/details/63687977)
- [「Coding Master」第30-35话](https://www.youtube.com/@tanyugang/videos)
- [Linux引导过程](https://zhuanlan.zhihu.com/p/608034586)
- [页表](https://github.com/freelancer-leon/notes/blob/master/kernel/mm/mm_pagetable.md)
- [四级分页下的页表自映射与基址随机化原理介绍](https://bbs.kanxue.com/thread-274152.htm)

```
# qemu monitor by `info registers`
CR0=80010033 CR2=0000000000000000 CR3=0000000007c01000 CR4=00000668
...
EFER=0000000000000d00
```

CR0 =1000 0000 0000 0001 0000 0000 0011 0011
CR4 =0000 0000 0000 0000 0000 0110 0110 1000
EFER=0000 0000 0000 0000 0000 1101 0000 0000

CR0.PE(bit 0) =1: 保护模式, 并使用分段机制
CR0.PG(bit 31) =1: 分页机制
CR4.PAE(bit 5) = 1: 启用PAE
IA32_EFER.LME(bit 8) = 1: 进入长模式
IA32_EFER.LMA(bit 10) = 1: 长模式激活: 上述如果条件全部满足, CPU会置 EFER.LMA = 1表示64位模式已激活

[Virtual to Physical Address Translation—Long Mode](/uefi/misc/img/longmode-virtual-to-physical-address-translation.png)
CR3 =0000 0000 0000 0000 0000 0000 0000 0000 0000 0111 1100 0000 0001 0000 0000 0000
当CR4[LA57]=1时表示使用5级分页, 因此uefi env使用4级分页, 地址空间是2^48(9-9-9-9-12)

PML4 地址= PML4 基地址(CR3[51:12]即高40位)+低位地址(PML4 表在 4 KB 边界上对齐, 低12位均为0)

```bash
(qemu) xp /5xg 0x7c01000                  
xp /5xg 0x7c01000
0000000007c01000: 0x0000000007c02023 0x0000000000000000 # PML4E
0000000007c01010: 0x0000000000000000 0x0000000000000000
0000000007c01020: 0x0000000000000000
(qemu) xp /5xg 0x7c02000
xp /5xg 0x7c02000
0000000007c02000: 0x0000000007c03023 0x0000000007c04003 # PDPE
0000000007c02010: 0x0000000007c05023 0x0000000007c06023
0000000007c02020: 0x0000000007c07003
(qemu) xp /5xg 0x0000000007c03000
xp /5xg 0x0000000007c03000
0000000007c03000: 0x00000000000000e3 0x0000000000200083 # PDE, 2M是没有PT即没有PTE
0000000007c03010: 0x0000000000400083 0x0000000000600083
0000000007c03020: 0x00000000008000e3
```

PML4 Addr = 0x7c01000 = PML4E(0) Addr
	PML4E只使用了一个

	PML4E(0) value = 0x0000000007c02023
PDP Addr =  0x0000000007c02000 = PDPE(0) Addr

	PDPE(0) value = 0x07c03023 = 0000 0111 1100 0000 0011 0000 0010 0011, PDPE.PS(bit 7) = 0

PD Addr = 0x0000000007c03000 = PDE(0) Addr
	
	PDE(0) value = 0x00000000000000e3 = 0000 0000 1110 0011, PDE.PS(bit 7) = 1

PDPE.PS=0 & PDE.PS = 1 => page size = 2M