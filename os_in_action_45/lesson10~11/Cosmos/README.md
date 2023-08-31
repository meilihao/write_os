# README
## 验证
```bash
cp ../lesson02/HelloOS/os.qcow2 HelloOS
make all ; 构建Cosmos.eki
pushd .
cd ../HelloOS
sudo ./setup.sh
popd
qemu-system-x86_64  -smp 4 -hda ../HelloOS/os.qcow2 -m 256 -enable-kvm
```

## FAQ
### `qemu-system-x86_64 -smp 4 -hda ../HelloOS/os.qcow2 -m 256 -enable-kvm`报`qemu-system-x86_64: warning: host doesn't support requested feature: CPUID.80000001H:ECX.svm [bit 2]`
ref:
- [svm is an AMD processor feature; the equivalent feature on Intel processors is known as vmx](https://unix.stackexchange.com/questions/710944/qemu-warning-host-doesnt-support-requested-feature-cpuid-80000001hecx-svm-b)

添加`-cpu host`

### `qemu-system-x86_64 -smp 4 -hda ../HelloOS/os.qcow2 -m 256 -enable-kvm`报`Your computer is not support ACPI!!`(from vm 屏幕)
为找到解决方法, 先注释`init_acpi()`

### `qemu-system-x86_64 -smp 4 -hda ../HelloOS/os.qcow2 -m 256 -enable-kvm`报`INITKRL DIE ERROR：not find file`(from vm 屏幕)
根据12课评论说是正常, 原因是没有内核文件(`Makefile中LKIMG_INFILE中是否已包含$(KRNLEXCIMG)`), 13课完成后即可解决
