// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr {
  uint magic;  // must equal ELF_MAGIC
  uchar elf[12];
  ushort type;
  ushort machine;
  uint version;
  uint entry;
  uint phoff;
  uint shoff;
  uint flags;
  ushort ehsize;
  ushort phentsize;
  ushort phnum;
  ushort shentsize;
  ushort shnum;
  ushort shstrndx;
};

// Program section header
// p_offset: 该segment的数据在文件中的偏移地址(相对文件头)
// p_vaddr: segment数据应该加载到进程的虚拟地址
// p_paddr: segment数据应该加载到进程的物理地址(如果对应系统使用的是物理地址)
// p_filesz: 该segment数据在文件中的大小
// p_memsz: 该segment数据在进程内存中的大小。注意需要满足p_memsz>=p_filesz，多出的部分初始化为0，通常作为.bss段内容
// p_flags: 进程中该segment的权限(R/W/X)
// p_align: 该segment数据的对齐，2的整数次幂。即要求p_offset % p_align = p_vaddr。
// 剩下的p_type字段，表示该program segment的类型，主要有
struct proghdr {
  uint type;
  uint off;
  uint vaddr;
  uint paddr;
  uint filesz;
  uint memsz;
  uint flags;
  uint align;
};

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4
