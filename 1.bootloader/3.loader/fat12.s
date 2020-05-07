.equ RootDirSectors, 14 # 根据FAT12文件系统提供的信息经过计算而得,即(BPB_RootEntCnt * 32 + BPB_BytesPerSec-1)/BPB_BytesPerSec = (224 × 32 + 512 -1)/512 =14
.equ SectorNumOfRootDirStart, 19 # boot扇区BPB_RsvdSecCnt+FAT table的扇区数BPB_FATSz16 * BPB_NumFATs = 1 + 9*2
.equ SectorNumOfFAT1Start,	1 # FAT table开始的扇区
.equ SectorDataStart,	33	 # SectorNumOfRootDirStart + RootDirSectors #因为FAT表的第0, 1两项是保留项, 因此有效簇号从2开始. 

BS_OEMName: .ascii "MINEboot"
BPB_BytesPerSec: .word	512
BPB_SecPerClus: .byte	1
BPB_RsvdSecCnt:	.word	1
BPB_NumFATs:	.byte	2
BPB_RootEntCnt:	.word	224
BPB_TotSec16:	.word	2880
BPB_Media:	.byte	0xf0
BPB_FATSz16: .word	9
BPB_SecPerTrk: .word	18
BPB_NumHeads:	.word	2
BPB_HiddSec: .int	0
BPB_TotSec32:	.int	0
BS_DrvNum: .byte	0
BS_Reserved1: .byte	0
BS_BootSig: .byte	0x29
BS_VolID: .int 0
BS_VolLab: .ascii "boot loader"
BS_FileSysType: .ascii "FAT12   "
