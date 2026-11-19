#!/usr/bin/env bash

set -ex

# `REAL_USER=$(id -un "$SUDO_UID")` + `runuser -u "$REAL_USER" -- bash -c 'qemu-system-x86_64 ...'`: 没用它, 无法将qemu-system-x86_64输出绑定到当前终端
# 可能需要按几下键盘才能进入uefi shell
qemu-system-x86_64 -machine q35 \
	-drive if=pflash,format=raw,readonly=on,file=OVMF_CODE.fd \
 	-drive if=pflash,format=raw,file=OVMF_VARS.fd \
 	-drive file=hda.img,format=raw,if=none,id=disk0 \
	-device ide-hd,drive=disk0,bus=ide.0 \
 	-device nec-usb-xhci,id=xhci \
    -device usb-mouse -device usb-kbd \
 	-smp 4 -m 256 -cpu host -enable-kvm \
 	-net none -serial stdio -monitor unix:/tmp/qemu.sock,server,nowait