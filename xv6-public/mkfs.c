#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define stat xv6_stat  // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
#include "stat.h"
#include "param.h"

#ifndef static_assert
#define static_assert(a, b) do { switch (0) case 0: case (a): ; } while (0)
#endif

#define NINODES 200

// Disk layout:
// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]

int nbitmap = FSSIZE/(BSIZE*8) + 1;
int ninodeblocks = NINODES / IPB + 1;
int nlog = LOGSIZE;
int nmeta;    // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks;  // Number of data blocks

int fsfd;
struct superblock sb;
char zeroes[BSIZE];
uint freeinode = 1;
uint freeblock;


void balloc(int);
void wsect(uint, void*);
void winode(uint, struct dinode*);
void rinode(uint inum, struct dinode *ip);
void rsect(uint sec, void *buf);
uint ialloc(ushort type);
void iappend(uint inum, void *p, int n);

// convert to intel byte order
ushort
xshort(ushort x)
{
  ushort y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  return y;
}

// 转小端
uint
xint(uint x)
{
  uint y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  a[2] = x >> 16;
  a[3] = x >> 24;
  return y;
}

// [xv6——文件系统：FS的布局和inode的读写操作 ](https://www.cnblogs.com/yinheyi/p/16464407.html)
// TODO: xv6 实现的不是现有的任何一个文件系统, 有人实现了minix v1: [Minix v1 文件系统的实现](https://silverrainz.me/blog/minix-v1-file-system.html)
int
main(int argc, char *argv[])
{
  int i, cc, fd;
  uint rootino, inum, off;
  struct dirent de;
  char buf[BSIZE];
  struct dinode din;


  static_assert(sizeof(int) == 4, "Integers must be 4 bytes!");

  if(argc < 2){
    fprintf(stderr, "Usage: mkfs fs.img files...\n");
    exit(1);
  }

  assert((BSIZE % sizeof(struct dinode)) == 0);
  assert((BSIZE % sizeof(struct dirent)) == 0);

  fsfd = open(argv[1], O_RDWR|O_CREAT|O_TRUNC, 0666);
  if(fsfd < 0){
    perror(argv[1]);
    exit(1);
  }

  // 1 fs block = 1 disk sector
  nmeta = 2 + nlog + ninodeblocks + nbitmap;
  nblocks = FSSIZE - nmeta; // FSSIZE=fs blocks; nmeta = meta blocks

  sb.size = xint(FSSIZE); // FSSIZE = 0x03E8
  sb.nblocks = xint(nblocks);
  sb.ninodes = xint(NINODES);
  sb.nlog = xint(nlog);
  sb.logstart = xint(2);
  sb.inodestart = xint(2+nlog);
  sb.bmapstart = xint(2+nlog+ninodeblocks);

  // nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
  printf("nmeta %d (boot, super, log blocks %u inode blocks %u, bitmap blocks %u) blocks %d total %d\n",
         nmeta, nlog, ninodeblocks, nbitmap, nblocks, FSSIZE);

  freeblock = nmeta;     // the first free block that we can allocate

  // 置零
  for(i = 0; i < FSSIZE; i++)
    wsect(i, zeroes);

  memset(buf, 0, sizeof(buf));
  memmove(buf, &sb, sizeof(sb));
  wsect(1, buf);

  // first sector is reserved for boot
  // # hexdump -n 1024 -C fs.img 
  // 00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  // *
  // 00000200  e8 03 00 00 ad 03 00 00  c8 00 00 00 1e 00 00 00  |................|
  // 00000210  02 00 00 00 20 00 00 00  3a 00 00 00 00 00 00 00  |.... ...:.......|
  // 00000220  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|

  rootino = ialloc(T_DIR);
  assert(rootino == ROOTINO);

  // `./..`均指向rootino
  bzero(&de, sizeof(de));
  de.inum = xshort(rootino);
  strcpy(de.name, ".");
  iappend(rootino, &de, sizeof(de));

  bzero(&de, sizeof(de));
  de.inum = xshort(rootino);
  strcpy(de.name, "..");
  iappend(rootino, &de, sizeof(de));

  for(i = 2; i < argc; i++){
    printf("name: [%d:%s]\n", i, argv[i]);
    assert(index(argv[i], '/') == 0); // char *index(const char *s, int c)用来找出参数s 字符串中第一个出现的参数c地址，然后将该字符出现的地址返回. 字符串结束字符(NULL)也视为字符串一部分

    if((fd = open(argv[i], 0)) < 0){
      perror(argv[i]);
      exit(1);
    }

    // Skip leading _ in name when writing to file system.
    // The binaries are named _rm, _cat, etc. to keep the
    // build operating system from trying to execute them
    // in place of system binaries like rm and cat.
    if(argv[i][0] == '_')
      ++argv[i];

    inum = ialloc(T_FILE);

    bzero(&de, sizeof(de));
    de.inum = xshort(inum);
    strncpy(de.name, argv[i], DIRSIZ);
    iappend(rootino, &de, sizeof(de));

    while((cc = read(fd, buf, sizeof(buf))) > 0)
      iappend(inum, buf, cc);

    close(fd);
  }

  // fix size of root inode dir
  // 修正理由: [类似linux(by `stat`), dir inode size是block size的整数倍](https://unix.stackexchange.com/questions/356682/how-directory-size-is-calculated).
  // 推测需要用dirent.inum=0来判断子文件条目结束
  rinode(rootino, &din);
  off = xint(din.size);
  printf("old rootino size: %d\n", off); // 288=de*18 = 16(program) + `.`+`..`
  off = ((off/BSIZE) + 1) * BSIZE; //TODO: 占用块数 * 块数. 当size刚好等于512时, off就多了, 应是(off+(BSIZE-1))/BSIZE*BSIZE
  din.size = xint(off);
  printf("new rootino size: %d\n", din.size);
  winode(rootino, &din);

  balloc(freeblock);

  exit(0);
}

void
wsect(uint sec, void *buf)
{
  if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){
    perror("lseek");
    exit(1);
  }
  if(write(fsfd, buf, BSIZE) != BSIZE){
    perror("write");
    exit(1);
  }
}

// write inode
void
winode(uint inum, struct dinode *ip)
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct dinode*)buf) + (inum % IPB); // 获取目标dinode的偏移地址
  *dip = *ip;
  wsect(bn, buf);
}

// read inode
void
rinode(uint inum, struct dinode *ip)
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct dinode*)buf) + (inum % IPB);
  *ip = *dip;
}

// read sector
void
rsect(uint sec, void *buf)
{
  if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){
    perror("lseek");
    exit(1);
  }
  if(read(fsfd, buf, BSIZE) != BSIZE){
    perror("read");
    exit(1);
  }
}

// 在磁盘上找到一空闲inode然后初始化并返回
uint
ialloc(ushort type)
{
  uint inum = freeinode++;
  struct dinode din;

  // bzero()将参数s 所指的内存区域前n 个字节全部设为零
  bzero(&din, sizeof(din));
  din.type = xshort(type);
  din.nlink = xshort(1);
  din.size = xint(0);
  winode(inum, &din);
  return inum;
}

/*
 * 作用： 依据块的使用情况写位图区
 * int : 已经使用的块的数量
 * 原理： 
 */
void
balloc(int used)
{
  uchar buf[BSIZE];
  int i;

  printf("balloc: first %d blocks have been allocated\n", used);
  assert(used < BSIZE*8);
  bzero(buf, BSIZE);
  for(i = 0; i < used; i++){
    buf[i/8] = buf[i/8] | (0x1 << (i%8));
  }
  printf("balloc: write bitmap block at sector %d\n", sb.bmapstart);
  wsect(sb.bmapstart, buf);
}

#define min(a, b) ((a) < (b) ? (a) : (b))

/*
 * [代码：i 节点内容](https://th0ar.gitbooks.io/xv6-chinese/content/content/chapter6.html)
 * 作用 ： 向inode里添加内容
 * inum : inode号
 * *xp : 要追加的信息的指针
 * n : 要追加的信息的大小
 * 原理 ： 1. 得到这个文件的大小，把它赋值给off变量
 *        2. 进而得到当前文件末尾的块编号fbn，进而得到分配的块号x。
 *        然后以buf为缓冲区，把不大于1个块的内容写入x号块。调整n,off,p的值，并持续循环，直到把所有的内容都写到块里。
 *        3. 修改inode的属性，并调用winode把inode的信息再写回磁盘。
 */
void
iappend(uint inum, void *xp, int n)
{
  char *p = (char*)xp;
  uint fbn, off, n1;
  struct dinode din;
  char buf[BSIZE];
  uint indirect[NINDIRECT];
  uint x;

  rinode(inum, &din);
  off = xint(din.size); // off文件偏移
  // printf("append inum %d at off %d sz %d\n", inum, off, n);
  while(n > 0){
    fbn = off / BSIZE; // fbn 文件占用块数
    assert(fbn < MAXFILE);
    if(fbn < NDIRECT){
      if(xint(din.addrs[fbn]) == 0){ // 没有数据块, 则分配
        din.addrs[fbn] = xint(freeblock++);
      }
      x = xint(din.addrs[fbn]);
    } else {
      if(xint(din.addrs[NDIRECT]) == 0){ // 没有间接块, 则分配
        din.addrs[NDIRECT] = xint(freeblock++);
      }
      rsect(xint(din.addrs[NDIRECT]), (char*)indirect); // 读取间接块内容
      if(indirect[fbn - NDIRECT] == 0){ // 没有间接块的数据块, 则分配, 再更新间接块
        indirect[fbn - NDIRECT] = xint(freeblock++);
        wsect(xint(din.addrs[NDIRECT]), (char*)indirect); 
      }
      x = xint(indirect[fbn-NDIRECT]);
    }
    n1 = min(n, (fbn + 1) * BSIZE - off); // (fbn + 1) * BSIZE - off: 扇区空闲空间; fbn + 1, 新数据写在哪个块数, 这里画图好理解(写游标在BSIZE窗口的某个位置)
    rsect(x, buf); // 读取扇区
    bcopy(p, buf + (off - (fbn * BSIZE)), n1); // 更新扇区内容. off - (fbn * BSIZE), 起始偏移. void bcopy(const void *src, void *dest, int n):将字符串src的前n个字节复制到dest中
    wsect(x, buf);
    n -= n1;
    off += n1;
    p += n1;
  }
  din.size = xint(off);
  winode(inum, &din);
}
