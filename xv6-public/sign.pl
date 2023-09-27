#!/usr/bin/perl

# 打开bootblock
open(SIG, $ARGV[0]) || die "open $ARGV[0]: $!";

# 将文件内容写到缓冲区当中, 读入最多 1000字节
$n = sysread(SIG, $buf, 1000);

if($n > 510){
  print STDERR "boot block too large: $n bytes (max 510)\n";
  exit 1;
}

print STDERR "boot block is $n bytes (max 510)\n";

# 没有超过的话, 在缓冲区尾部一直填充0, 然后第 511个字节必须为 0x55, 第512个字节必须为 0xAA
$buf .= "\0" x (510-$n);
$buf .= "\x55\xAA";

open(SIG, ">$ARGV[0]") || die "open >$ARGV[0]: $!"; # 大于字符表现写: 如果文件不存在, 就会被创立; 如果文件存在, 则覆盖写
print SIG $buf; # 将缓冲区内容写回文件
close SIG;
