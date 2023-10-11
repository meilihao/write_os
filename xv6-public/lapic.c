// The local APIC manages internal (non-I/O) interrupts.
// See Chapter 8 & Appendix C of Intel processor manual volume 3.

#include "param.h"
#include "types.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "traps.h"
#include "mmu.h"
#include "x86.h"

// Local APIC registers, divided by 4 for use as uint[] indices.
// 寄存器的偏移需要除以4，用作lapic数组的下标来访问寄存器
#define ID      (0x0020/4)   // ID
#define VER     (0x0030/4)   // Version
#define TPR     (0x0080/4)   // Task Priority
#define EOI     (0x00B0/4)   // EOI
#define SVR     (0x00F0/4)   // Spurious Interrupt Vector
  #define ENABLE     0x00000100   // Unit Enable
#define ESR     (0x0280/4)   // Error Status
#define ICRLO   (0x0300/4)   // Interrupt Command
  #define INIT       0x00000500   // INIT/RESET
  #define STARTUP    0x00000600   // Startup IPI
  #define DELIVS     0x00001000   // Delivery status
  #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
  #define DEASSERT   0x00000000
  #define LEVEL      0x00008000   // Level triggered
  #define BCAST      0x00080000   // Send to all APICs, including self.
  #define BUSY       0x00001000
  #define FIXED      0x00000000
#define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
#define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
  #define X1         0x0000000B   // divide counts by 1
  #define PERIODIC   0x00020000   // Periodic
#define PCINT   (0x0340/4)   // Performance Counter LVT
#define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
#define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
#define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
  #define MASKED     0x00010000   // Interrupt masked
#define TICR    (0x0380/4)   // Timer Initial Count
#define TCCR    (0x0390/4)   // Timer Current Count
#define TDCR    (0x03E0/4)   // Timer Divide Configuration

volatile uint *lapic;  // Initialized in mp.c

//PAGEBREAK!
static void
lapicw(int index, int value)
{
  lapic[index] = value;
  lapic[ID];  // wait for write to finish, by reading
}

// [Xv6内核分析(三.3)](http://www.databusworld.cn/9296.html)
// lapic寄存器的描述可以在Intel开发手册第三卷第10章找到
// lapic寄存器空间总共有4KB大小，被映射到0xFEE00000开始的物理地址空间
// 访问lapic寄存器时，寄存器的地址必须是16字节对齐的，每次访问一般是4字节访问
void
lapicinit(void)
{
  if(!lapic)
    return;

  // Enable local APIC; set spurious interrupt vector.
  lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS)); // 设置SVR寄存器

  // The timer repeatedly counts down at bus frequency
  // from lapic[TICR] and then issues an interrupt.
  // If xv6 cared more about precise timekeeping,
  // TICR would be calibrated using an external time source.
  // 设置lapic定时器
  lapicw(TDCR, X1);
  lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER)); // timer mode被设置为了periodic; 中断向量号被设置为了T_IRQ0 + IRQ_TIMER
  lapicw(TICR, 10000000);

  // Disable logical interrupt lines.
  // LINT0和LINT1这两个寄存器，它们对应到Local APIC模块的INTR和NMI引脚，外部的中断会引发这两个寄存器的中断发起. xv6不使用它们
  lapicw(LINT0, MASKED);
  lapicw(LINT1, MASKED);

  // Disable performance counter overflow interrupts
  // on machines that provide that interrupt entry.
  // 大于等于4，表示当前cpu支持性能计数中断，否则不支持。如果支持，则操作PCINT寄存器屏蔽此中断
  if(((lapic[VER]>>16) & 0xFF) >= 4)
    lapicw(PCINT, MASKED);

  // Map error interrupt to IRQ_ERROR.
  lapicw(ERROR, T_IRQ0 + IRQ_ERROR);

  // Clear error status register (requires back-to-back writes).
  // 向此寄存器写值可以更新错误寄存器的状态，所以连续写两次寄存器就相当于清除状态寄存器了. 另外此寄存器时可读的，但是读之前必须要先写此寄存器
  lapicw(ESR, 0);
  lapicw(ESR, 0);

  // Ack any outstanding interrupts.
  lapicw(EOI, 0);

  // Send an Init Level De-Assert to synchronise arbitration ID's.
  // 设置ICR中断命令寄存器，从而设置IPI中断属性
  // ICR寄存器是用来设置核间中断（IPI）的属性的
  lapicw(ICRHI, 0);
  lapicw(ICRLO, BCAST | INIT | LEVEL);
  while(lapic[ICRLO] & DELIVS) // 设置后等待Delivery Status为空闲状态
    ;

  // Enable interrupts on the APIC (but not on the processor).
  // 设置TPR任务优先级. 打开 CPU 的 LAPIC 的中断，这使得 LAPIC 能够将中断传递给本地处理器
  lapicw(TPR, 0);
}

// 读取lapic空间中的ID寄存器，然后获得lapicid
int
lapicid(void)
{
  if (!lapic)
    return 0;
  return lapic[ID] >> 24;
}

// Acknowledge interrupt.
void
lapiceoi(void)
{
  if(lapic)
    lapicw(EOI, 0);
}

// Spin for a given number of microseconds.
// On real hardware would want to tune this dynamically.
void
microdelay(int us)
{
}

#define CMOS_PORT    0x70
#define CMOS_RETURN  0x71

// Start additional processor running entry code at addr.
// See Appendix B of MultiProcessor Specification.
// BSP通过向AP逐个发送中断来启动AP，首先发送INIT中断来初始化AP，然后发送SIPI中断来启动AP，发送中断使用的是写ICR寄存器的方式
void
lapicstartap(uchar apicid, uint addr)
{
  int i;
  ushort *wrv;

  // 设置CMOS的shutdown code寄存器，设置从核运行的code地址
  // "The BSP must initialize CMOS shutdown code to 0AH
  // and the warm reset vector (DWORD based at 40:67) to point at
  // the AP startup code prior to the [universal startup algorithm]."
  outb(CMOS_PORT, 0xF);  // offset 0xF is shutdown code
  outb(CMOS_PORT+1, 0x0A);
  wrv = (ushort*)P2V((0x40<<4 | 0x67));  // Warm reset vector // 设置warm reset vector，warm reset vector表示从核启动的地址，warm reset vector本身所在的物理地址是0x467
  wrv[0] = 0;
  wrv[1] = addr >> 4;

  // "Universal startup algorithm."
  // Send INIT (level-triggered) interrupt to reset other CPU.
  // 发送INIT中断以重置AP
  // 中断命令寄存器（ICR）是一个 64 位本地 APIC寄存器，允许运行在处理器上的软件指定和发送处理器间中断（IPI）给系统中的其它处理器
  lapicw(ICRHI, apicid<<24); // 将目标CPU的ID写入ICR寄存器的目的地址域中
  lapicw(ICRLO, INIT | LEVEL | ASSERT); // 在ASSERT的情况下将INIT中断写入ICR寄存器
  microdelay(200);
  lapicw(ICRLO, INIT | LEVEL); // //在非ASSERT的情况下将INIT中断写入ICR寄存器
  microdelay(100);    // should be 10ms, but too slow in Bochs! // 等待100ms (INTEL官方手册规定的是10ms,但是由于Bochs运行较慢，此处改为100ms)

  // Send startup IPI (twice!) to enter code.
  // Regular hardware is supposed to only accept a STARTUP
  // when it is in the halted state due to an INIT.  So the second
  // should be ignored, but it is part of the official Intel algorithm.
  // Bochs complains about the second one.  Too bad for Bochs.
  // INTEL官方规定发送两次startup IPI中断
  for(i = 0; i < 2; i++){
    lapicw(ICRHI, apicid<<24); // 将目标CPU的ID写入ICR寄存器的目的地址域中
    lapicw(ICRLO, STARTUP | (addr>>12)); // 将SIPI中断写入ICR寄存器的传送模式域中，将启动代码写入向量域中. 这个物理地址一定要是4KB对齐的，所以addr右移了12位
    microdelay(200);
  }
}

#define CMOS_STATA   0x0a
#define CMOS_STATB   0x0b
#define CMOS_UIP    (1 << 7)        // RTC update in progress

#define SECS    0x00
#define MINS    0x02
#define HOURS   0x04
#define DAY     0x07
#define MONTH   0x08
#define YEAR    0x09

static uint
cmos_read(uint reg)
{
  outb(CMOS_PORT,  reg);
  microdelay(200);

  return inb(CMOS_RETURN);
}

static void
fill_rtcdate(struct rtcdate *r)
{
  r->second = cmos_read(SECS);
  r->minute = cmos_read(MINS);
  r->hour   = cmos_read(HOURS);
  r->day    = cmos_read(DAY);
  r->month  = cmos_read(MONTH);
  r->year   = cmos_read(YEAR);
}

// qemu seems to use 24-hour GWT and the values are BCD encoded
void
cmostime(struct rtcdate *r)
{
  struct rtcdate t1, t2;
  int sb, bcd;

  sb = cmos_read(CMOS_STATB);

  bcd = (sb & (1 << 2)) == 0;

  // make sure CMOS doesn't modify time while we read it
  for(;;) {
    fill_rtcdate(&t1);
    if(cmos_read(CMOS_STATA) & CMOS_UIP)
        continue;
    fill_rtcdate(&t2);
    if(memcmp(&t1, &t2, sizeof(t1)) == 0)
      break;
  }

  // convert
  if(bcd) {
#define    CONV(x)     (t1.x = ((t1.x >> 4) * 10) + (t1.x & 0xf))
    CONV(second);
    CONV(minute);
    CONV(hour  );
    CONV(day   );
    CONV(month );
    CONV(year  );
#undef     CONV
  }

  *r = t1;
  r->year += 2000;
}
