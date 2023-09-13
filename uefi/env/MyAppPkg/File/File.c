#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/DiskIo.h>
#include <Guid/FileInfo.h>
#include <Library/DevicePathLib.h>


EFI_STATUS PrintNode(EFI_DEVICE_PATH_PROTOCOL * Node)
{
	Print(L"(%d %d)/", Node->Type, Node->SubType);
	return 0;
}

EFI_DEVICE_PATH_PROTOCOL* WalkthroughDevicePath(EFI_DEVICE_PATH_PROTOCOL* DevPath, EFI_STATUS (*Callbk)(EFI_DEVICE_PATH_PROTOCOL*))
{
	EFI_DEVICE_PATH_PROTOCOL* pDevPath = DevPath;
	while(!IsDevicePathEnd (pDevPath))
	{
        if(Callbk)
		{
			EFI_STATUS Status = Callbk(pDevPath);
			if(Status != 0) 
			{
				if(Status < 0) pDevPath = NULL;
				break;
			}
		}
		pDevPath = NextDevicePathNode (pDevPath); 
	}
	return pDevPath;
}


// 系统中的每个设备都有一个唯一的路径
// 设备路径中的节点称为设备节点，设备路径由设备节点组成的列表构成，列表由结束设备节点结束
// 每个设备节点都以EFI_DEVICE_PATH_PROTOCOL开始，结束设备节点是一个特设的设备节点，它的类型为0x7F，次类型为0xFF或0x01，节点长度为2字节
// UEFI提供了EFI_DEVICE_PATH_TO_TEXT_PROTOCOL用于将设备路径转换为字符串，其中的成员函数ConvertDevicePathToText用于将设备路径DevicePath转换为字符串
// IsDevicePathEnd（CONST VOID *Node）用于判断设备节点Node是否为设备路径的设备结束节点
// NextDevicePathNode（CONST VOID *Node）用于返回设备节点Node的下一个设备节点
EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS          Status  = EFI_SUCCESS;
	UINTN               HandleIndex,NumHandles;
	EFI_HANDLE *ControllerHandle =NULL;

	EFI_LOADED_IMAGE        *LoadedImage;
	EFI_DEVICE_PATH         *DevicePath;
	EFI_FILE_IO_INTERFACE   *Vol;
	EFI_FILE_HANDLE         RootFs;
	EFI_FILE_HANDLE         FileHandle;
	CHAR16* TextDevicePath;


	EFI_DEVICE_PATH_TO_TEXT_PROTOCOL* Device2TextProtocol = 0;

	gBS->LocateProtocol(&gEfiDevicePathToTextProtocolGuid,NULL,(VOID**)&Device2TextProtocol);


    //找出所有支持DiskIo的设备, 比如ata控制器, 具体分区(没有未分区磁盘)
	Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiDiskIoProtocolGuid, NULL, &NumHandles, &ControllerHandle);
	if (EFI_ERROR(Status)) {
			Print(L"No Disk\n");
			return Status;
	}
	Print(L"Found Num: %d\n", NumHandles);

	//遍历每个DiskIo设备，并打开设备上的DevicePathprotocol
  	EFI_DEVICE_PATH_PROTOCOL *DiskDevicePath;
	for(HandleIndex=0;HandleIndex<NumHandles;HandleIndex++){
			Status = gBS->OpenProtocol(ControllerHandle[HandleIndex],
										&gEfiDevicePathProtocolGuid,
										(VOID**)&DiskDevicePath,
										gImageHandle,
										NULL,
										EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	
			if (EFI_ERROR(Status)){
					continue;
			}  
	
			TextDevicePath = Device2TextProtocol->ConvertDevicePathToText(DiskDevicePath,TRUE,TRUE);
			Print(L"TextDevicePath:%s\n",TextDevicePath);

			if(TextDevicePath)
			{
				gBS->FreePool(TextDevicePath);
			}

		//遍历设备路径DiskDevicePath里的各个设备节点
		WalkthroughDevicePath(DiskDevicePath,PrintNode);
		Print(L"\n\n");
	}

	Print(L"---\n\n");

	gBS->HandleProtocol(ImageHandle,&gEfiLoadedImageProtocolGuid,(VOID*)&LoadedImage);
	// LoadedImage->DeviceHandle: 加载 EFI 映像的设备句柄
	gBS->HandleProtocol(LoadedImage->DeviceHandle,&gEfiDevicePathProtocolGuid,(VOID*)&DevicePath);

	TextDevicePath = Device2TextProtocol->ConvertDevicePathToText(DevicePath,FALSE,TRUE); 
	Print(L"TextDevicePath:%s\n",TextDevicePath);
	if(TextDevicePath)
		gBS->FreePool(TextDevicePath);
	WalkthroughDevicePath(DevicePath,PrintNode); 
	Print(L"\n");

	gBS->HandleProtocol(LoadedImage->DeviceHandle,&gEfiSimpleFileSystemProtocolGuid,(VOID*)&Vol);
	Vol->OpenVolume(Vol,&RootFs);
	RootFs->Open(RootFs,&FileHandle,(CHAR16*)L"kernel.bin",EFI_FILE_MODE_READ,0);

	EFI_FILE_INFO* FileInfo;
	UINTN BufferSize = 0;
	EFI_PHYSICAL_ADDRESS pages = 0x100000;

	BufferSize = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 100;
	gBS->AllocatePool(EfiRuntimeServicesData,BufferSize,(VOID**)&FileInfo); 
	FileHandle->GetInfo(FileHandle,&gEfiFileInfoGuid,&BufferSize,FileInfo);
	Print(L"\tFileName:%s\t Size:%d\t FileSize:%d\t Physical Size:%d\n",FileInfo->FileName,FileInfo->Size,FileInfo->FileSize,FileInfo->PhysicalSize);

	Print(L"Read kernel file to memory\n");
	gBS->AllocatePages(AllocateAddress,EfiLoaderData,(FileInfo->FileSize + 0x1000 - 1) / 0x1000,&pages);
	BufferSize = FileInfo->FileSize;
	FileHandle->Read(FileHandle,&BufferSize,(VOID*)pages);
	gBS->FreePool(FileInfo);
	FileHandle->Close(FileHandle);
	RootFs->Close(RootFs);

	gBS->CloseProtocol(LoadedImage->DeviceHandle,&gEfiSimpleFileSystemProtocolGuid,ImageHandle,NULL);
	gBS->CloseProtocol(LoadedImage->DeviceHandle,&gEfiDevicePathProtocolGuid,ImageHandle,NULL);
	gBS->CloseProtocol(ImageHandle,&gEfiLoadedImageProtocolGuid,ImageHandle,NULL);
	gBS->CloseProtocol(Device2TextProtocol,&gEfiDevicePathToTextProtocolGuid,ImageHandle,NULL);

	return EFI_SUCCESS;
}

