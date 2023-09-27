#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Protocol/BlockIo.h>
#include  <Protocol/LoadedImage.h>
#include  <Protocol/SimpleFileSystem.h>
#include  <Library/DevicePathLib.h>
#include  <Guid/FileInfo.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Library/BaseMemoryLib.h>
#include "elf.h"
#include "relocate.h"
#include "file_loader.h"

// Ehdr Address:6216000
// Phdr Address:6216034
// Start Address:100000
// End Address:108000
// Start Address:8010798E ??? xv6@lastes is 0x00108000, 通过`ld -Map=output.map`发现是entry.S导致
// End Address:80193000
// Allocate Start Address:100000
// Allocate End Address:80193000
// Entry Address:10000C
// AllocatePages=524436 // 需要2G空间, qemu默认是128
// Could not allocate pages at 07F105E0, error: 14
EFI_STATUS RelocateELF(CHAR16* KernelPath, EFI_PHYSICAL_ADDRESS* RelocateAddr){
  EFI_STATUS Status;
  EFI_PHYSICAL_ADDRESS BufferAddress;
  UINTN BufferPageSize;
  Status = LoadFile(KernelPath,&BufferAddress,&BufferPageSize);
  if(EFI_ERROR(Status)){
    Print(L"Failed to Load File: %s\n",KernelPath);
    return Status;
  }
  Elf32_Ehdr *Ehdr = (Elf32_Ehdr*)BufferAddress;
  Elf32_Phdr *Phdr = (Elf32_Phdr*)(BufferAddress + Ehdr->e_phoff);
  Print(L"Ehdr Address:%x\n",Ehdr);
  Print(L"Phdr Address:%x\n",Phdr);
  UINTN i;
  EFI_PHYSICAL_ADDRESS alloc_start_address;
  EFI_PHYSICAL_ADDRESS alloc_end_address;
  UINT8 init = 0;
  for(i=0; i<Ehdr->e_phnum;i++){
    if(Phdr[i].p_type == PT_LOAD){
      EFI_PHYSICAL_ADDRESS start_address = Phdr[i].p_paddr;
      EFI_PHYSICAL_ADDRESS end_address = start_address + Phdr[i].p_memsz;
      UINTN mask = Phdr[i].p_align-1;
      end_address = (end_address + mask) & ~mask;
      Print(L"Start Address:%x\n",start_address);
      Print(L"End Address:%x\n",end_address);
      if(init == 0){
        alloc_start_address = start_address;
        alloc_end_address = end_address;
        init = 1;
      }else{
        if(start_address < alloc_start_address) alloc_start_address = start_address;
        if(end_address > alloc_end_address) alloc_end_address = end_address;
      }
    }
  }
  
  Print(L"Allocate Start Address:%x\n",alloc_start_address);
  Print(L"Allocate End Address:%x\n",alloc_end_address);
  Print(L"Entry Address:%x\n",Ehdr->e_entry);
  UINTN page_size = ((alloc_end_address - alloc_start_address)/4096)+1;
  Print(L"AllocatePages=%d\n",page_size);
  Status = gBS->AllocatePages(
    AllocateAddress,
    EfiLoaderData,
    page_size,
    &alloc_start_address);
  if (EFI_ERROR(Status)) {
    Print(L"Could not allocate pages at %08lx, error: %d \n", &alloc_start_address, Status);
    return Status;
  }
  for(i=0; i<Ehdr->e_phnum; i++){
    if(Phdr[i].p_type == PT_LOAD){
      EFI_PHYSICAL_ADDRESS start_address = Phdr[i].p_paddr;
      EFI_PHYSICAL_ADDRESS end_address = start_address + Phdr[i].p_memsz;
      UINTN mask = Phdr[i].p_align-1;
      end_address = (end_address + mask) & ~mask;
      CopyMem((VOID *)start_address,(VOID *)(BufferAddress + Phdr[i].p_offset),Phdr[i].p_filesz);
      if(Phdr[i].p_memsz > Phdr[i].p_filesz){
        SetMem((VOID *)(start_address + Phdr[i].p_filesz),Phdr[i].p_memsz - Phdr[i].p_filesz,0);
        Print(L"Zero Clear at %x - %x\n",start_address + Phdr[i].p_filesz,end_address);
      }
        Print(L"Relocated at %x - %x\n",start_address,end_address);
    }
  }
  *RelocateAddr = (EFI_PHYSICAL_ADDRESS)Ehdr->e_entry;
  FreePages((VOID *)BufferAddress,BufferPageSize);
  
  return Status;
}
