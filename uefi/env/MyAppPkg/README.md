# README

## build
```bash
cd MyAppPkg
build -p MyAppPkg/MyAppPkg.dsc -a X64 # `-p`是以edk2目录为根
```

## example
- [一个UEFI引导程序的实现](https://www.ituring.com.cn/book/2763)

    - Memory
    - Video
    - VideoSet
    - File

## FAQ
### File/directory not found in workspace
没有指定env PACKAGES_PATH, 获取其包含相对路径

### `Module for [X64] is not a component of active platform`
1. dsc中的component块中没有指定inf文件的路径导致编译的时候dsc文件和inf文件不匹配，这个时候要往dsc文件中的component块中添加inf文件相对于udk 根目录的路径即可
2. build时没指定`-p xxx.dsc`

### `Instance of library class [RegisterFilterLib] is not found`
在edk2中搜索`RegisterFilterLib`, 找到后加入dsc的`LibraryClasses`