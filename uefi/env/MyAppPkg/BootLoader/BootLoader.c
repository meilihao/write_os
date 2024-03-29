/***************************************************
*		版权声明
*
*	本操作系统名为：MINE
*	该操作系统未经授权不得以盈利或非盈利为目的进行开发，
*	只允许个人学习以及公开交流使用
*
*	代码最终所有权及解释权归田宇所有；
*
*	本模块作者：	田宇
*	EMail:		345538255@qq.com
*
*
***************************************************/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>

struct EFI_GRAPHICS_OUTPUT_INFORMATION
{
	unsigned int HorizontalResolution;
	unsigned int VerticalResolution;
	unsigned int PixelsPerScanLine;

	unsigned long FrameBufferBase;
	unsigned long FrameBufferSize;
};

struct EFI_E820_MEMORY_DESCRIPTOR
{
	unsigned long address;
	unsigned long length;
	unsigned int  type;
}__attribute__((packed));

struct EFI_E820_MEMORY_DESCRIPTOR_INFORMATION
{
	unsigned int E820_Entry_count;
	struct EFI_E820_MEMORY_DESCRIPTOR E820_Entry[0];
};

struct KERNEL_BOOT_PARAMETER_INFORMATION
{
	struct EFI_GRAPHICS_OUTPUT_INFORMATION Graphics_Info;
	struct EFI_E820_MEMORY_DESCRIPTOR_INFORMATION E820_Info;
};

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_LOADED_IMAGE        *LoadedImage; // 加载的image信息
	EFI_FILE_IO_INTERFACE   *Vol; // 可理解为存储设备上的分区
	EFI_FILE_HANDLE         RootFs; //指向所打开的卷的根目录的指针
	EFI_FILE_HANDLE         FileHandle;

	int i = 0;
	void (*func)(void);
	EFI_STATUS status = EFI_SUCCESS;
	struct KERNEL_BOOT_PARAMETER_INFORMATION *kernel_boot_para_info = NULL;

////////////////////// loader kernel
	gBS->HandleProtocol(ImageHandle,&gEfiLoadedImageProtocolGuid,(VOID*)&LoadedImage);
	gBS->HandleProtocol(LoadedImage->DeviceHandle,&gEfiSimpleFileSystemProtocolGuid,(VOID*)&Vol);
	Vol->OpenVolume(Vol,&RootFs);
	status = RootFs->Open(RootFs,&FileHandle,(CHAR16*)L"kernel.bin",EFI_FILE_MODE_READ,0);
	if(EFI_ERROR(status))
	{
		Print(L"Open kernel.bin Failed.\n");
		return status;
	}

	EFI_FILE_INFO* FileInfo;
	UINTN BufferSize = 0;
	EFI_PHYSICAL_ADDRESS pages = 0x100000; // 1MB

	BufferSize = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 100;
	gBS->AllocatePool(EfiRuntimeServicesData,BufferSize,(VOID**)&FileInfo); 
	FileHandle->GetInfo(FileHandle,&gEfiFileInfoGuid,&BufferSize,FileInfo);
	Print(L"\tFileName:%s\t Size:%d\t FileSize:%d\t Physical Size:%d\n",FileInfo->FileName,FileInfo->Size,FileInfo->FileSize,FileInfo->PhysicalSize);

	gBS->AllocatePages(AllocateAddress,EfiLoaderData,(FileInfo->FileSize + 0x1000 - 1) / 0x1000,&pages); // 0x1000=4k, 分配的首地址
	Print(L"Read Kernel File to Memory Address:%018lx\n",pages);
	BufferSize = FileInfo->FileSize;
	FileHandle->Read(FileHandle,&BufferSize,(VOID*)pages);
	gBS->FreePool(FileInfo);
	FileHandle->Close(FileHandle);
	RootFs->Close(RootFs);

/////////////////// 初始化kernel_boot_para_info + 设置显示器mode & map Graphics FrameBufferBase
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gGraphicsOutput = 0;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info = 0;
	UINTN InfoSize = 0;

	pages = 0x60000; // 144k [Memory Map (x86)](https://wiki.osdev.org/Memory_Map_(x86))
	kernel_boot_para_info = (struct KERNEL_BOOT_PARAMETER_INFORMATION *)0x60000; //给kernel传递参数
	gBS->AllocatePages(AllocateAddress,EfiLoaderData,1,&pages);
	gBS->SetMem((void*)kernel_boot_para_info,0x1000,0); //初始化

	gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,NULL,(VOID **)&gGraphicsOutput);

	long H_V_Resolution = gGraphicsOutput->Mode->Info->HorizontalResolution * gGraphicsOutput->Mode->Info->VerticalResolution;
	int MaxResolutionMode = gGraphicsOutput->Mode->Mode; // 获取到的最大分辨率

	for(i = 0;i < gGraphicsOutput->Mode->MaxMode;i++)
	{
		gGraphicsOutput->QueryMode(gGraphicsOutput,i,&InfoSize,&Info);
		if((Info->PixelFormat == 1) && (Info->HorizontalResolution * Info->VerticalResolution > H_V_Resolution))
		{
			H_V_Resolution = Info->HorizontalResolution * Info->VerticalResolution;
			MaxResolutionMode = i;
		}
		gBS->FreePool(Info);
	}

	gGraphicsOutput->SetMode(gGraphicsOutput,MaxResolutionMode);
	gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,NULL,(VOID **)&gGraphicsOutput);
	Print(L"Current Mode:%02d,Version:%x,Format:%d,Horizontal:%d,Vertical:%d,ScanLine:%d,FrameBufferBase:%018lx,FrameBufferSize:%018lx\n",gGraphicsOutput->Mode->Mode,gGraphicsOutput->Mode->Info->Version,gGraphicsOutput->Mode->Info->PixelFormat,gGraphicsOutput->Mode->Info->HorizontalResolution,gGraphicsOutput->Mode->Info->VerticalResolution,gGraphicsOutput->Mode->Info->PixelsPerScanLine,gGraphicsOutput->Mode->FrameBufferBase,gGraphicsOutput->Mode->FrameBufferSize);

	kernel_boot_para_info->Graphics_Info.HorizontalResolution = gGraphicsOutput->Mode->Info->HorizontalResolution;
	kernel_boot_para_info->Graphics_Info.VerticalResolution = gGraphicsOutput->Mode->Info->VerticalResolution;
	kernel_boot_para_info->Graphics_Info.PixelsPerScanLine = gGraphicsOutput->Mode->Info->PixelsPerScanLine;
	kernel_boot_para_info->Graphics_Info.FrameBufferBase = gGraphicsOutput->Mode->FrameBufferBase;
	kernel_boot_para_info->Graphics_Info.FrameBufferSize = gGraphicsOutput->Mode->FrameBufferSize;

	Print(L"Map Graphics FrameBufferBase to Virtual Address 0xffff800003000000\n");
	// 需要配合head.S理解, 使用0x200000=2^21=2MB物理页, 即设置gGraphicsOutput FrameBuffer的页表
	long * PageTableEntry = (long *)0x103000; // 1M+12k
	for(i = 0;i < (gGraphicsOutput->Mode->FrameBufferSize + 0x200000 - 1) >> 21;i++)	// map to virtual address 0xffff800003000000
	{
		*(PageTableEntry + 24 + i) = gGraphicsOutput->Mode->FrameBufferBase | 0x200000 * i | 0x87;
		Print(L"Page %02d,Address:%018lx,Value:%018lx\n",i,(long)(PageTableEntry + 24 + i),*(PageTableEntry + 24 + i));
	}

/////////////////////////// 获取内存map
	struct EFI_E820_MEMORY_DESCRIPTOR *E820p = kernel_boot_para_info->E820_Info.E820_Entry;
	struct EFI_E820_MEMORY_DESCRIPTOR *LastE820 = NULL;
	unsigned long LastEndAddr = 0;
	int E820Count = 0;

	UINTN MemMapSize = 0; // 存放mem map buffer大小
	EFI_MEMORY_DESCRIPTOR* MemMap = 0;
	UINTN MapKey = 0;
	UINTN DescriptorSize = 0; // 每个mem map描述符大小
	UINT32 DesVersion = 0;

	gBS->GetMemoryMap(&MemMapSize,MemMap,&MapKey,&DescriptorSize,&DesVersion);
	MemMapSize += DescriptorSize * 5;
	gBS->AllocatePool(EfiRuntimeServicesData,MemMapSize,(VOID**)&MemMap);
	Print(L"Get MemMapSize:%d,MapKey:%d,DescriptorSize:%d,count:%d\n",MemMapSize,MapKey,DescriptorSize,MemMapSize/DescriptorSize);
	gBS->SetMem((void*)MemMap,MemMapSize,0);
	status = gBS->GetMemoryMap(&MemMapSize,MemMap,&MapKey,&DescriptorSize,&DesVersion);
	Print(L"Get MemMapSize:%d,MapKey:%d,DescriptorSize:%d,count:%d\n",MemMapSize,MapKey,DescriptorSize,MemMapSize/DescriptorSize); // MemMapSize可能和上面GetMemoryMap获取的不一样
	if(EFI_ERROR(status))
		Print(L"status:%018lx\n",status);

	Print(L"Get EFI_MEMORY_DESCRIPTOR Structure:%018lx\n",MemMap);
	for(i = 0;i < MemMapSize / DescriptorSize;i++)
	{
		int MemType = 0;
		EFI_MEMORY_DESCRIPTOR* MMap = (EFI_MEMORY_DESCRIPTOR*) ((CHAR8*)MemMap + i * DescriptorSize);
		if(MMap->NumberOfPages == 0)
			continue;
//		Print(L"MemoryMap %4d %10d (%16lx<->%16lx) %016lx\n",MMap->Type,MMap->NumberOfPages,MMap->PhysicalStart,MMap->PhysicalStart + (MMap->NumberOfPages << 12),MMap->Attribute);
		switch(MMap->Type)
		{
			case EfiReservedMemoryType:
			case EfiMemoryMappedIO:
			case EfiMemoryMappedIOPortSpace:
			case EfiPalCode:
				MemType= 2;	//2:ROM or Reserved
				break;

			case EfiUnusableMemory:
				MemType= 5;	//5:Unusable
				break;

			case EfiACPIReclaimMemory:
				MemType= 3;	//3:ACPI Reclaim Memory
				break;

			case EfiLoaderCode:
			case EfiLoaderData:
			case EfiBootServicesCode:
			case EfiBootServicesData:
			case EfiRuntimeServicesCode:
			case EfiRuntimeServicesData:
			case EfiConventionalMemory:
			case EfiPersistentMemory:
				MemType= 1;	//1:RAM
				break;

			case EfiACPIMemoryNVS:
				MemType= 4;	//4:ACPI NVS Memory
				break;

			default:
				Print(L"Invalid UEFI Memory Type:%4d\n",MMap->Type);
				continue;
		}

		if((LastE820 != NULL) && (LastE820->type == MemType) && (MMap->PhysicalStart == LastEndAddr))
		{
			LastE820->length += MMap->NumberOfPages << 12;
			LastEndAddr += MMap->NumberOfPages << 12;
		}
		else
		{
			E820p->address = MMap->PhysicalStart;
			E820p->length = MMap->NumberOfPages << 12;
			E820p->type = MemType;
			LastEndAddr = MMap->PhysicalStart + (MMap->NumberOfPages << 12); // PhysicalStart + NumberOfPages *4k
			LastE820 = E820p;
			E820p++;
			E820Count++;			
		}
	}

	kernel_boot_para_info->E820_Info.E820_Entry_count = E820Count;
	LastE820 = kernel_boot_para_info->E820_Info.E820_Entry; // E820_Entry is first E820p
	// sort E820_Info: 升序
	int j = 0;
	for(i = 0; i< E820Count; i++)
	{
		struct EFI_E820_MEMORY_DESCRIPTOR* e820i = LastE820 + i; // LastE820 + i*sizeof(EFI_E820_MEMORY_DESCRIPTOR), 编译器会自动处理地址相加(类似`LastE820++`)???
		struct EFI_E820_MEMORY_DESCRIPTOR MemMap;
		for(j = i + 1; j< E820Count; j++)
		{
			struct EFI_E820_MEMORY_DESCRIPTOR* e820j = LastE820 + j;
			if(e820i->address > e820j->address)
			{
				MemMap = *e820i;
				*e820i = *e820j;
				*e820j = MemMap;
			}
		}
	}

	LastE820 = kernel_boot_para_info->E820_Info.E820_Entry;
	for(i = 0;i < E820Count;i++)
	{
		Print(L"MemoryMap (%10lx<->%10lx) %4d\n",LastE820->address,LastE820->address+LastE820->length,LastE820->type);
		LastE820++;
	}
	gBS->FreePool(MemMap);

	Print(L"Call ExitBootServices And Jmp to Kernel.\n");
	gBS->GetMemoryMap(&MemMapSize,MemMap,&MapKey,&DescriptorSize,&DesVersion); // for ExitBootServices's MapKey ???
	Print(L"Get MemMapSize:%d,MapKey:%d,DescriptorSize:%d,count:%d\n",MemMapSize,MapKey,DescriptorSize,MemMapSize/DescriptorSize); // 本代码三次调用GetMemoryMap, 返回的MemMapSize和MapKey都不同, 推测与中间调用了内存分配有关, 比如AllocatePool,FreePool

	gBS->CloseProtocol(LoadedImage->DeviceHandle,&gEfiSimpleFileSystemProtocolGuid,ImageHandle,NULL);
	gBS->CloseProtocol(ImageHandle,&gEfiLoadedImageProtocolGuid,ImageHandle,NULL);

	gBS->CloseProtocol(gGraphicsOutput,&gEfiGraphicsOutputProtocolGuid,ImageHandle,NULL);

	return EFI_SUCCESS; // for debug

///////////////////// 退出ExitBootServices + 开始执行kernel
	status = gBS->ExitBootServices(ImageHandle,MapKey);
	if(EFI_ERROR(status))
	{
		Print(L"ExitBootServices: Failed, Memory Map has Changed.\n");
		return EFI_INVALID_PARAMETER;
	}
	func = (void *)0x100000; // kernel.bin的地址
	func();

	return EFI_SUCCESS;
}
