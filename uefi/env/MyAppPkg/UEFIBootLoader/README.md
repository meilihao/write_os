# README
ref:
- [UEFIBootLoader](https://gitee.com/MINEOS_admin/publish)
- [jiebaomaster/sOS64/tree/master/src/UEFIbootloader](https://github.com/jiebaomaster/sOS64/tree/master/src/UEFIbootloader)

	其README有部分UEFIbootloader的排错信息和注意事项
- [0x06 从零开始开发操作系统 UEFI](https://zhuanlan.zhihu.com/p/683894156)

修改了BootLoader.inf里的BaseName, 因与[`BootLoader/BootLoader.inf`](../BootLoader/BootLoader.inf)重复了.

## build
```bash
export PACKAGES_PATH=/home/chen/git/write_os/uefi/env/edk2:$(pwd)
build -p MyAppPkg/MyAppPkg.dsc -a X64 -m  UEFIBootLoader/BootLoader.inf # `-p`是以edk2目录为根
```

## 测试
```bash
# pwd
/home/chen/git/write_os/uefi/env/MyAppPkg
# cp /home/chen/git/write_os/uefi/env/edk2/Build/MyAppPkg/DEBUG_GCC5/X64/MyAppPkg/UEFIBootLoader/BootLoader/OUTPUT/UEFIBootLoader.efi ../root2
# qemu-system-x86_64 -machine q35 -bios /usr/share/qemu/OVMF.fd -drive format=raw,file=fat:rw:root2 -net none
> FS0: # 进入UEFI shell
> UEFIBootLoader.efi
```

### U盘测试
ref:
- [U盘UEFI引导OS内核的小白教程](https://blog.csdn.net/weixin_44391390/article/details/113459555)

```bash
mount /dev/sdb1 /mnt/ # sdb1是u盘制作的ESP分区
mkdir /mnt/EFI/BOOT
cp UEFIBootLoader.efi /mnt/EFI/BOOT/BOOTx64.EFI # efi文件放在/mnt/EFI/BOOT/目录下，且命名为BOOTx64.EFI的目的是，UFEI可以开机时自动执行该efi并引导内核
cp kernel.bin /mnt # kernel.bin在根目录下
sync
# -- 参考[scripts/copy_bins.sh](https://gitee.com/MINEOS_admin/publish/blob/develop/scripts/copy_bins.sh), 拷入os二进制, 部分二进制与copy_bins.sh里的不一致
cp /home/chen/tmp/publish-develop/kernel/{kernel.bin,system} /mnt/ # [MINEOS_admin/publish/kernel](https://gitee.com/MINEOS_admin/publish/tree/develop/kernel)执行make构建的代码
cp cp /home/chen/tmp/publish-develop/user/{init.bin,system_api_lib} root2 /mnt/ # [MINEOS_admin/publish/user](https://gitee.com/MINEOS_admin/publish/tree/develop/user)执行make构建的代码
umount /mnt/
```