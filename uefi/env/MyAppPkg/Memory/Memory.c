// [UEFI 基础教程 （十五） - 获取UEFI MemoryMap](https://blog.csdn.net/xiaopangzi313/article/details/109928878)
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

// uefi shell: memmap可打印mem region, 见[MemMap.c](ShellPkg/Library/UefiShellDebug1CommandsLib/MemMap.c)
EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable)
{

	struct MemoryMap {
		EFI_MEMORY_DESCRIPTOR* MemMap; // 指向缓冲区
		UINTN MemMapSize; // GetMemoryMap()返回的MemoryMapSize
		UINTN MapKey; // 用于标识内存映射的值, 内存映射的内容会随着引导服务的使用而发生变化. edk2 的实现中，MapKey 本质上是一个单调递增的计数器，每次内存映射更新时都会增加
		UINTN DescriptorSize; // 每个EFI_MEMORY_DESCRIPTOR的大小, 无法使用sizeof(EFI_MEMORY_DESCRIPTOR), 因为某些uefi实现可能有扩展
		UINT32 DesVersion;
	};

	struct MemoryMap memmap = { NULL, 0, 0, 0, 0};

	Print(L"Get EFI_MEMORY_DESCRIPTOR Structure\n");
	EFI_STATUS status = gBS->GetMemoryMap(&memmap.MemMapSize,memmap.MemMap,&memmap.MapKey,&memmap.DescriptorSize,&memmap.DesVersion); // 获取MemMapSize
	if(status != EFI_BUFFER_TOO_SMALL)
	{
		Print(L"GetMemoryMap failed, status = %r\n", status);
		return status;
	}

	Print(L"need mem size = %d\n", memmap.MemMapSize);

	memmap.MemMapSize += 4 * memmap.DescriptorSize; // 预留余量（2*(一段内存分割后, 最多多出2段内存)）
	gBS->AllocatePool(EfiRuntimeServicesData,memmap.MemMapSize,(VOID**)&memmap.MemMap);      // 按MemMapSize分配MemMap
	status = gBS->GetMemoryMap(&memmap.MemMapSize,memmap.MemMap,&memmap.MapKey,&memmap.DescriptorSize,&memmap.DesVersion); // 填充MemMap
	if(EFI_ERROR(status))
	{
		Print(L"GetMemoryMap failed, status = %r\n", status);
		return status;
	}

	Print(L"mem info: mem size = %d, item size = %d, count: %d, version: %d\n",
		 memmap.MemMapSize, memmap.DescriptorSize, memmap.MemMapSize / memmap.DescriptorSize, memmap.DesVersion);

	for(int i = 0; i< memmap.MemMapSize / memmap.DescriptorSize; i++)
	{
		EFI_MEMORY_DESCRIPTOR* MMap = (EFI_MEMORY_DESCRIPTOR*) (((CHAR8*)memmap.MemMap) + i * memmap.DescriptorSize);
		Print(L"MemoryMap %4d %10d (%10lx~%10lx) %016lx\n",
			MMap->Type,MMap->NumberOfPages,MMap->PhysicalStart,MMap->PhysicalStart + (MMap->NumberOfPages << 12),MMap->Attribute);
	}
	gBS->FreePool(memmap.MemMap);

	return EFI_SUCCESS;
}

// 内存类型:
// - reserved: Reserved memory
// - LoaderCode: Loader code
// - LoaderData: Loader data
// - BS_code: Boot service code
// - BS_data: Boot service data
//
// 	BS_code, BS_data可能很多, 每块占用不大, 在ExitBootServices后会回收
// - RT_data: Runtime data
// - `ACPI_*`: acpi
// - mmio
// - available: Available memory

// EfiConventionalMemory: Available memory space
// EfiLoaderCode: UEFI Application code
// EfiLoaderData: UEFI Application data
// EfiBootServicesCode: UEFI/PEI/DXE driver
// EfiBootServicesData: UEFI/PEI/DXE driver allocated memory
// EfiPersistentMemory
// EfiRuntimeServicesCode: Runtime driver. It can’t be used by OS.
// EfiRuntimeServicesData: Runtime data. It can’t be used by OS.
// EfiReservedMemoryType: Reserved memory in BIOS. It can’t be used by OS.
// EfiACPIReclaimMemory: Used by ACPI.
// EfiACPIMemoryNVS: Used by ACPI. It can’t be used by OS.
// EfiMemoryMappedIO
// EfiMemoryMappedIOPortSpace