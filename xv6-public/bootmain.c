// Boot loader.
//
// Part of the boot block, along with bootasm.S, which calls bootmain().
// bootasm.S has put the processor into protected 32-bit mode.
// bootmain() loads an ELF kernel image from the disk starting at
// sector 1 and then jumps to the kernel entry routine.

#include "types.h"
#include "elf.h"
#include "x86.h"
#include "memlayout.h"

#define SECTSIZE  512

void readseg(uchar*, uint, uint);

//将内核的ELF文件从硬盘加载进内存，并将控制权转给内核程序
// [Xv6内核分析(一)](http://www.databusworld.cn/9213.html)
void
bootmain(void)
{
  struct elfhdr *elf;
  struct proghdr *ph, *eph;
  void (*entry)(void);
  uchar* pa;

  elf = (struct elfhdr*)0x10000;  // scratch space, 64k

  // Read 1st page off disk
  // ELF 头不会超过4k
  readseg((uchar*)elf, 4096, 0);

  // Is this an ELF executable?
  if(elf->magic != ELF_MAGIC)
    return;  // let bootasm.S handle error

  // Load each program segment (ignores ph flags).
  // 解析elf文件，将代码段和数据段等信息拷贝到对应的加载地址处.
  // 拷贝段到物理地址 phoff
  ph = (struct proghdr*)((uchar*)elf + elf->phoff); // pht起始地址
  eph = ph + elf->phnum; // pht结束地址
  for(; ph < eph; ph++){
    pa = (uchar*)ph->paddr;
    readseg(pa, ph->filesz, ph->off);
    if(ph->memsz > ph->filesz)
      stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
  }

  // Call the entry point from the ELF header.
  // Does not return!
  entry = (void(*)(void))(elf->entry);
  entry();
}

void
waitdisk(void)
{
  // Wait for disk ready.
  // inb 表示读取硬件的端口，0x1F7 读操作时作为状态寄存器，这是一个8位寄存器，第7个bit表示的是 DRDY 位，表示磁盘就行，检测正常可以执行命令。第8个bit表示的是 BSY 位，表示磁盘是否繁忙
  // 0xC0=1100 0000
  // 0x40=0100 0000
  while((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

// Read a single sector at offset into dst.
// 读取扇区内容到dst
// [  第一节 读写IDE设备](https://www.askpure.com/course_U9FD8SB9-G9T99WXM-3RULAGMG-HH8I7UTF.html)
void
readsect(void *dst, uint offset)
{
  // Issue command.
  // use LBA28
  // 0x1F6 的高四位表示LAB模式，0xE0 = 0x1110000. 高四位中除了最后一个bit全部为1，最后一个bit为0表示是主盘（因为主盘是 xv6.img，从盘是 fs.img）
  waitdisk();
  outb(0x1F2, 1);   // count = 1
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0);
  outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

  // Read data.
  waitdisk();
  insl(0x1F0, dst, SECTSIZE/4);
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
void
readseg(uchar* pa, uint count, uint offset)
{
  uchar* epa;

  epa = pa + count; // 内存中的终止地址

  // Round down to sector boundary.
  // 让物理地址内存对齐, 这样方便加载, pa 就一定是扇区大小的倍数
  pa -= offset % SECTSIZE;

  // Translate from bytes to sectors; kernel starts at sector 1.
  // 读取的起始扇区
  offset = (offset / SECTSIZE) + 1;

  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}
