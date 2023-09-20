#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>


EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable)
{
	UINTN MemMapSize = 0;
	EFI_MEMORY_DESCRIPTOR* MemMap = 0;
	UINTN MapKey = 0;
	UINTN DescriptorSize = 0;
	UINT32 DesVersion = 0;

	Print(L"Get EFI_MEMORY_DESCRIPTOR Structure\n");
	gBS->GetMemoryMap(&MemMapSize,MemMap,&MapKey,&DescriptorSize,&DesVersion); // 获取MemMapSize
	gBS->AllocatePool(EfiRuntimeServicesData,MemMapSize,(VOID**)&MemMap);      // 按MemMapSize分配MemMap
	gBS->GetMemoryMap(&MemMapSize,MemMap,&MapKey,&DescriptorSize,&DesVersion); // 填充MemMap

	int i = 0;
	for(i = 0; i< MemMapSize / DescriptorSize; i++)
	{
		EFI_MEMORY_DESCRIPTOR* MMap = (EFI_MEMORY_DESCRIPTOR*) (((CHAR8*)MemMap) + i * DescriptorSize);
		Print(L"MemoryMap %4d %10d (%10lx~%10lx) %016lx\n",MMap->Type,MMap->NumberOfPages,MMap->PhysicalStart,MMap->PhysicalStart + (MMap->NumberOfPages << 12),MMap->Attribute);
	}
	gBS->FreePool(MemMap);

	return EFI_SUCCESS;
}

内存类型:
- reserved: Reserved memory
- LoaderCode: Loader code
- LoaderData: Loader data
- BS_code: Boot service code
- BS_data: Boot service data

	BS_code, BS_data可能很多, 每块占用不大, 在ExitBootServices后会回收
- RT_data: Runtime data
- `ACPI_*`: acpi
- mmio
- available: Available memory
