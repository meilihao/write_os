# README

## edk2
ref:
- [UEFI 基础教程 （一） - 运行第一个APP HelloWorld](https://blog.csdn.net/weixin_41028621/article/details/112546820)
- [**My First UEFI Application**](https://davysouza.medium.com/my-first-uefi-application-486f319cf4a4)
	target是arm

	code: [hello-world-uefi](https://github.com/davysouza/hello-world-uefi)

edk2环境设置见`programming-interface/arch/uefi.md`.

## 测试
### fs
ref:
- [UEFI Hello World with qemu and EDK2](https://ursache.io/posts/uefi-hello-world-2023/)
- [Getting started with EFI](https://krinkinmu.github.io/2020/10/11/efi-getting-started.html)

**本方式容易测试uefi应用, 推荐**

```bash
mkdir -p root2/efi/boot
cp /usr/lib/grub/x86_64-efi/monolithic/grubx64.efi root2/efi/boot/bootx64.efi # 模拟cd, 硬盘启动的路径是`boot/efi`
qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -drive format=raw,file=fat:rw:root2 -net none [-serial stdio] # `-serial stdio`需要构建ovmf时添加`-D DEBUG_ON_SERIAL_PORT`
```

### disk
```bash
# qemu-img create -f raw hda.img 64M # 使用raw便于查看disk的内容
# modprobe nbd max_part=8
# ./setup.sh # 创建hda.img
# ./deploy.sh # 设置hda.img
# qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -machine q35 -smp 4 -hda hda.img -m 256 -enable-kvm -cpu host [-net none] # 可能需要按几下键盘才能进入uefi shell
```