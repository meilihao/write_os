# README
```bash
gcc HelloWorld.c -E -o  HelloWorld.i # 预处理：加入头文件，替换宏
gcc HelloWorld.c -S -c -o HelloWorld.s # 编译：包含预处理，将 C 程序转换成汇编程序
gcc HelloWorld.c -c -o HelloWorld.o # 汇编：包含预处理和编译，将汇编程序转换成可链接的二进制程序
gcc HelloWorld.c -o HelloWorld # 链接：包含以上所有操作，将可链接的二进制程序和其它别的库链接在一起，形成可执行的程序文件
objdump -d HelloWorld # 反汇编
```