#ifndef ELF_H_
#define ELF_H_
#include <stdint.h>

#define EI_NIDENT 16

typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Size;

// # readelf -h image/kernel 
// ELF Header:
//   Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
//   Class:                             ELF32
//   Data:                              2's complement, little endian
//   Version:                           1 (current)
//   OS/ABI:                            UNIX - System V
//   ABI Version:                       0
//   Type:                              EXEC (Executable file)
//   Machine:                           Intel 80386
//   Version:                           0x1
//   Entry point address:               0x10000c
//   Start of program headers:          52 (bytes into file)
//   Start of section headers:          710096 (bytes into file)
//   Flags:                             0x0
//   Size of this header:               52 (bytes)
//   Size of program headers:           32 (bytes)
//   Number of program headers:         3
//   Size of section headers:           40 (bytes)
//   Number of section headers:         19
//   Section header string table index: 18
// # hexdump -C -n 128 image/kernel
// 00000000  7f 45 4c 46 01 01 01 00  00 00 00 00 00 00 00 00  |.ELF............| //e_ident(16)
// 00000010  02 00 03 00 01 00 00 00  0c 00 10 00 34 00 00 00  |............4...| // e_type(2)+e_machine(2)+e_version(4);e_entry(4)+e_phoff(4)
// 00000020  d0 d5 0a 00 00 00 00 00  34 00 20 00 03 00 28 00  |........4. ...(.| // e_shoff(4)+e_flags(4); e_ehsize(2)+e_phentsize(2)+e_phnum(2)+e_shentsize(2)
// 00000030  13 00 12 00 01 00 00 00  00 10 00 00 00 00 10 80  |................| // e_shnum(2)+e_shstrndx(2)
// [ELF文件解析二](https://dawuge.github.io/2019/06/05/2016-06-05-elf-fs-2/)
typedef struct {
	unsigned char e_ident[EI_NIDENT]; // 文件标识，一般为“\x7fELF”，通过这四位，可以查看该文件是否为ELF文件
	Elf32_Half e_type; // ELF文件的类型，如ET_REL(1)表示可重定位文件，ET_EXEC(2)表示可执行文件，ET_DYN(3)表示共享目标文件
	Elf32_Half e_machine; // arch类型, 3=EM_386
	Elf32_Word e_version; // elf文件版本
	Elf32_Addr e_entry; //执行入口, 没有时为0
	Elf32_Off  e_phoff; // program header table的offset，如果文件没有PH，这个值是0
	Elf32_Off  e_shoff; // section header table 的offset，如果文件没有SH，这个值是0
	Elf32_Word e_flags; // 特定于处理器的标志，32位和64位Intel架构都没有定义标志，因此eflags的值是0
	Elf32_Half e_ehsize; // ELF header的大小，32位ELF是52字节，64位是64字节
	Elf32_Half e_phentsize; // program header table中每个入口的大小
	Elf32_Half e_phnum; // 如果文件没有program header table, e_phnum的值为0。e_phentsize乘以e_phnum就得到了整个program header table的大小. `readelf -l image/kernel`
	Elf32_Half e_shentsize; // section header table中entry的大小，即每个section header占多少字节
	Elf32_Half e_shnum; // section header table中header的数目。如果文件没有section header table, e_shnum的值为0。e_shentsize乘以e_shnum，就得到了整个section header table的大小
	Elf32_Half e_shstrndx; // section header string table index. 包含了section header table中section name string table。如果没有section name string table, e_shstrndx的值是SHN_UNDEF
} Elf32_Ehdr;

typedef struct {
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off  sh_offset;
	Elf32_Size sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Size sh_addralign;
	Elf32_Size sh_entsize;
} Elf32_Shdr;

typedef struct {
	Elf32_Word p_type;
	Elf32_Off  p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Size p_filesz;
	Elf32_Size p_memsz;
	Elf32_Word p_flags;
	Elf32_Size p_align;
} Elf32_Phdr;

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2

typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint64_t Elf64_Size;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

typedef struct {
	unsigned char e_ident[EI_NIDENT];
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;
	Elf64_Off  e_phoff;
	Elf64_Off  e_shoff;
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct {
	Elf64_Word sh_name;
	Elf64_Word sh_type;
	Elf64_Xword sh_flags;
	Elf64_Addr sh_addr;
	Elf64_Off  sh_offset;
	Elf64_Size sh_size;
	Elf64_Word sh_link;
	Elf64_Word sh_info;
	Elf64_Size sh_addralign;
	Elf64_Size sh_entsize;
} Elf64_Shdr;

typedef struct {
	Elf64_Word p_type;
	Elf64_Word p_flags;
	Elf64_Off  p_offset;
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr;
	Elf64_Size p_filesz;
	Elf64_Size p_memsz;
	Elf64_Size p_align;
} Elf64_Phdr;
#endif 
