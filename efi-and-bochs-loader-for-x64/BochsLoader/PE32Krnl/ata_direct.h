// Copyright by Xiyue87 2022

#include "ctype.h"

#ifndef _INCLUDE_ATA_DIRECT_H_
#define _INCLUDE_ATA_DIRECT_H_

#define SECTOR_SIZE             512
#define ATA_0_BASE              (0x1F0)

#define ATA_COMMAND             (7)
#define ATA_STATUS              (7)
#define ATA_DATA                (0)

#define ATA_SECTOR_COUNT        (2)
#define ATA_SECTOR_0            (3)
#define ATA_SECTOR_1            (4)
#define ATA_SECTOR_2            (5)
#define ATA_SECTOR_3            (6) /* only low 4-bit used */

#define ATA_STATUS_BUSY         0x80 
#define ATA_STATUS_READY        0x40 
#define ATA_STATUS_DMAR         0x20 
#define ATA_STATUS_SERV         0x10 
#define ATA_STATUS_DATAREADY    0x08 
#define ATA_STATUS_ERR          0x01 
#define ATA_STATUS_CHK          0x01

#define ATA_LBA_MODE            0xE0

void test_unit_ready();
void test_pio_ready();
int read_sector(char* buffer, uint32_t lba);

#endif