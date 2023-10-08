// Multiprocessor support
// Search memory for MP description structures.
// http://developer.intel.com/design/pentium/datashts/24201606.pdf

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mp.h"
#include "x86.h"
#include "mmu.h"
#include "proc.h"

struct cpu cpus[NCPU];
int ncpu;
uchar ioapicid;

static uchar
sum(uchar *addr, int len)
{
  int i, sum;

  sum = 0;
  for(i=0; i<len; i++)
    sum += addr[i];
  return sum;
}

// Look for an MP structure in the len bytes at addr.
static struct mp*
mpsearch1(uint a, int len)
{
  uchar *e, *p, *addr;

  addr = P2V(a);
  e = addr+len;
  for(p = addr; p < e; p += sizeof(struct mp))
    if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0) // 整个结构的字节加起来结果为0(规范要求)
      return (struct mp*)p;
  return 0;
}

// Search for the MP Floating Pointer Structure, which according to the
// spec is in one of the following three locations:
// 1) in the first KB of the EBDA;
// 2) in the last KB of system base memory;
// 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
// 寻找mp floating pointer 结构, 与查找acpi rsdp类似(mp floating pointer在acpi rsdp里)
// [实例讲解多处理器下(xv6)的计算机启动](https://zhuanlan.zhihu.com/p/394247844)
// [Memory Map (x86)](https://wiki.osdev.org/Memory_Map_(x86)), [BDA - BIOS Data Area - PC Memory Map](https://stanislavs.org/helppc/bios_data_area.html)
static struct mp*
mpsearch(void)
{
  uchar *bda;
  uint p;
  struct mp *mp;

  bda = (uchar *) P2V(0x400);
  if((p = ((bda[0x0F]<<8)| bda[0x0E]) << 4)){ // 在EBDA中最开始1K中寻找
    if((mp = mpsearch1(p, 1024)))
      return mp;
  } else {
    p = ((bda[0x14]<<8)|bda[0x13])*1024; // 在系统base mem的最后1K中查找
    if((mp = mpsearch1(p-1024, 1024)))
      return mp;
  }
  return mpsearch1(0xF0000, 0x10000); // 在BOIS ROM地址空间中(0F0000h 到 0FFFFFh)
}

// Search for an MP configuration table.  For now,
// don't accept the default configurations (physaddr == 0).
// Check for correct signature, calculate the checksum and,
// if correct, check the version.
// To do: check extended table checksum.
static struct mpconf*
mpconfig(struct mp **pmp)
{
  struct mpconf *conf;
  struct mp *mp;

  if((mp = mpsearch()) == 0 || mp->physaddr == 0)
    return 0;
  conf = (struct mpconf*) P2V((uint) mp->physaddr);
  if(memcmp(conf, "PCMP", 4) != 0)
    return 0;
  if(conf->version != 1 && conf->version != 4)
    return 0;
  if(sum((uchar*)conf, conf->length) != 0)
    return 0;
  *pmp = mp;
  return conf;
}

// [MultiProcessor Specification](https://en.wikipedia.org/wiki/MultiProcessor_Specification)
// xv6使用了MPS. 这也是它在qemu 6.2上仅使用一个cpu的原因: [QEMU 不会生成旧版 MP 表](https://www.mail-archive.com/qemu-discuss@nongnu.org/msg07249.html)
// 由于大多数较新的机器都支持包含MPS功能的高级配置和电源接口（ACPI），因此MPS在很大程度上已被ACPI取代. MPS仍可用于计算机或不支持ACPI的操作系统
// 因此MP表基本上已经过时了. ACPI MADT表是配置多处理器系统的真正信息来源
// [IntelMP（Intel Multiple Processor）相关结构](https://blog.csdn.net/stupid_haiou/article/details/46430749)
void
mpinit(void)
{
  uchar *p, *e;
  int ismp;
  struct mp *mp;
  struct mpconf *conf;
  struct mpproc *proc;
  struct mpioapic *ioapic;

  if((conf = mpconfig(&mp)) == 0)
    panic("Expect to run on an SMP");
  ismp = 1;
  lapic = (uint*)conf->lapicaddr;
  for(p=(uchar*)(conf+1), e=(uchar*)conf+conf->length; p<e; ){ //跳过表头，从第一个表项开始for循环
    switch(*p){
    case MPPROC: // 如果是处理器
      proc = (struct mpproc*)p;
      if(ncpu < NCPU) {
        cpus[ncpu].apicid = proc->apicid;  // apicid may differ from ncpu
        ncpu++;
      }
      p += sizeof(struct mpproc); // len is 20B
      continue;
    case MPIOAPIC:
      ioapic = (struct mpioapic*)p;
      ioapicid = ioapic->apicno;
      p += sizeof(struct mpioapic);
      continue;
    case MPBUS:
    case MPIOINTR:
    case MPLINTR:
      p += 8;
      continue;
    default:
      ismp = 0;
      break;
    }
  }
  if(!ismp)
    panic("Didn't find a suitable machine");

  if(mp->imcrp){ // 如果cpu支持imcr功能，那么就设置IMCR寄存器，退出PIC Mode，强制所有中断都经过IO APIC
    // Bochs doesn't support IMCR, so this doesn't run on Bochs.
    // But it would on real hardware.
    outb(0x22, 0x70);   // Select IMCR
    outb(0x23, inb(0x23) | 1);  // Mask external interrupts. // setting the IMCR 0x1 is one step jumping from PIC mode to Symmetric I/O mode, making the CPU receiving IRQs from local APIC, not directly from 8259A PIC
  }

  cprintf("ncpu %d\n", ncpu);
}
