// Intel 8250 serial port (UART).
// Intel 8250串行端口（UART）

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

#define COM1    0x3f8 // 定义第一个串口

static int uart;    // is there a uart?

// [XV6学习（8）中断和设备驱动](https://www.cnblogs.com/weijunji/p/xv6-study-8.html)
// 对Intel 8250 serial port (UART)设备进行初始化
// riscv版驱动调用的UART硬件是由QEMU模拟的16550芯片，一个16550芯片可以管理一条连接到终端或其他电脑的RS232串行链路。在QEMU中，其连接到键盘和显示器. 见[S081——设备中断与驱动部分(串口驱动与Console)——xv6源码完全解析系列(7)](https://blog.csdn.net/zzy980511/article/details/131288968)
void
uartinit(void)
{
  char *p;

  // Turn off the FIFO
  outb(COM1+2, 0); // 向端口com1+2 写0

  // 9600 baud, 8 data bits, 1 stop bit, parity off.
  outb(COM1+3, 0x80);    // Unlock divisor
  outb(COM1+0, 115200/9600);
  outb(COM1+1, 0);
  outb(COM1+3, 0x03);    // Lock divisor, 8 data bits.
  outb(COM1+4, 0);
  outb(COM1+1, 0x01);    // Enable receive interrupts.

  // If status is 0xFF, no serial port. //如果状态为0xFF，则无串行端口
  if(inb(COM1+5) == 0xFF) // COM1+5端口用于读入串口数据
    return;
  uart = 1;

  // Acknowledge pre-existing interrupt conditions;
  // enable interrupts.
  inb(COM1+2);
  inb(COM1+0);
  ioapicenable(IRQ_COM1, 0); // 调用ioapicenable使能了uart中断

  // Announce that we're here.
  for(p="xv6...\n"; *p; p++)
    uartputc(*p);
}

// 串口写入
void
uartputc(int c)
{
  int i;

  if(!uart)
    return;
  for(i = 0; i < 128 && !(inb(COM1+5) & 0x20); i++)
    microdelay(10);
  outb(COM1+0, c); // 端口0x3f8 写入c
}

static int
uartgetc(void)
{
  if(!uart)
    return -1;
  if(!(inb(COM1+5) & 0x01))
    return -1;
  return inb(COM1+0); // 返回端口0x3f8读取的数据
}

void
uartintr(void)
{
  consoleintr(uartgetc);
}
