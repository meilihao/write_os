// Copyright by Xiyue87 2022

#include "ctype.h"
#include "sys32.h"
#include "ata_direct.h"

void test_unit_ready()
{
    while ((io_in_byte(ATA_0_BASE + ATA_STATUS) & ATA_STATUS_READY) == 0)
    {
        asm_delay();
    }

    return;
}

void test_pio_ready()
{
    while ((io_in_byte(ATA_0_BASE + ATA_STATUS) & ATA_STATUS_DATAREADY) == 0)
    {
        asm_delay();
    }

    return;
}
int read_sector(char* buffer, uint32_t lba)
{
    uint8_t out_data;
    test_unit_ready();

    io_out_byte(ATA_0_BASE + ATA_SECTOR_COUNT, 1);
    asm_delay();
    io_out_byte(ATA_0_BASE + ATA_SECTOR_0, lba & 0xFF);
    asm_delay();
    io_out_byte(ATA_0_BASE + ATA_SECTOR_1, (lba >> 8) & 0xFF);
    asm_delay();
    io_out_byte(ATA_0_BASE + ATA_SECTOR_2, (lba >> 16) & 0xFF);
    asm_delay();
    out_data = (lba >> 24) & 0x0F; // only use low 4-bit
    out_data = out_data | ATA_LBA_MODE; // 1 1 1 0 -> Bit 7, 5 always 1, Bit 6: 1-LBA/0-CHS
    io_out_byte(ATA_0_BASE + ATA_SECTOR_3, out_data);
    asm_delay();

    io_out_byte(ATA_0_BASE + ATA_COMMAND, 0x20);

    test_pio_ready();
    io_in_word_string(ATA_0_BASE + ATA_DATA, (uint16_t*)buffer, SECTOR_SIZE / sizeof(uint16_t));

    return 0;
}