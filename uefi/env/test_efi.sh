#!/usr/bin/env bash

set -ex

if [ $# -gt 0 ]
then
	EFI_FILE=$1
    echo "using <efi path>: $EFI_FILE"

	\cp -f $EFI_FILE root2 # root2/EFI/BOOT/BOOTX64.EFI
	sync

	sleep 3 # 避免qemu-system-x86_64启动并接管终端后, 终端被清屏
fi

# 异常情况:
# 1. `cp $EFI_FILE root2/EFI/BOOT/BOOTX64.EFI`时因为EFI_FILE中没有阻塞代码(比如死循环), 导致uefi固件自动执行完成BOOTX64.EFI后返回到`Please select boot device`界面, 
#	 此时BOOTX64.EFI可能的打印内容会仅一闪就消失, 现象看起来就像BOOTX64.EFI根本没执行. 
#	 而在`Please select boot device`界面多次选择`UEFI QEMU HARDDISK QM0001`(即`file=fat:rw:root2`)并执行可以在终端或qemu窗口观察到一闪而逝的打印内容.
#    建议将测试的efi文件拷贝到root2/EFI, 再进uefi shell手动执行代码并查看打印内容
# 
qemu-system-x86_64 -machine q35 \
	-drive if=pflash,format=raw,readonly=on,file=OVMF_CODE.fd \
 	-drive if=pflash,format=raw,file=OVMF_VARS.fd \
 	-drive format=raw,file=fat:rw:root2 \
 	-net none -serial stdio -monitor unix:/tmp/qemu.sock,server,nowait