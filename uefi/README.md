# README

## 环境说明
1. `.clangd`

    - `-fshort-wchar`: 将 wchar_t 类型从默认的 4 字节（32 位）改为 2 字节（16 位）, 因为 wchar_t 在 Linux 等系统上默认是 32 位的, 而UEFI 规范规定字符串使用 UTF-16 编码，设置后`L"Hello"`这样的字符串字面量就会生成 UTF-16 编码的数组，可以直接传给 UEFI 函数使用
1. `env/test_efi.sh`: 测试构建的 EFI 文件