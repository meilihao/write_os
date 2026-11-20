#/***************************************************
#		版权声明
#
#	本操作系统名为：MINE
#	该操作系统未经授权不得以盈利或非盈利为目的进行开发，
#	只允许个人学习以及公开交流使用
#
#	代码最终所有权及解释权归田宇所有；
#
#	本模块作者：	田宇
#	EMail:		345538255@qq.com
#
#
#***************************************************/

all: head.o
	# 把一个 64 位 ELF 目标文件 head.o 转换成纯二进制镜像 kernel.bin，同时剥离掉符号表和调试信息，并移除 .eh_frame 和 .comment 这两个节
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary head.o kernel.bin

head.o: head.s
	as --64 -o head.o head.s

clean:
	rm -rf head.o kernel.bin

