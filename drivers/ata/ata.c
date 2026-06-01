#include <drivers/ata.h>
#include <kernel/ports.h>
#include <kernel/printk.h>


static int ata_wait_bsy() {
    uint32_t timeout = 1000000;
    while (timeout--) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (status == 0xFF) return -1; // floating buss
        if (!(status & ATA_STATUS_BSY)) return 0;
    }
    return -1; // Timeout
}

static int ata_wait_drq() {
    uint32_t timeout = 1000000;
    while (timeout--) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (status & ATA_STATUS_ERR) return -1; // disk (err)
        if (status & ATA_STATUS_DRQ) return 0;
    }
    return -1; // Timeout
}


void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    if (ata_wait_bsy() != 0) return;

    outb(ATA_PRIMARY_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    
    for(int i=0; i<4; i++) inb(ATA_PRIMARY_STATUS);

    outb(ATA_PRIMARY_SECCOUNT, 1);
    outb(ATA_PRIMARY_LBA_LOW,  (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_PIO);

    if (ata_wait_bsy() != 0) return;
    if (ata_wait_drq() != 0) return;

    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inw(ATA_PRIMARY_DATA);
    }
}

void ata_write_sector(uint32_t lba, uint8_t* buffer) {
    ata_wait_bsy();

    outb(ATA_PRIMARY_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, 1);
    outb(ATA_PRIMARY_LBA_LOW,  (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HIGH, (uint8_t)(lba >> 16));

    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_PIO);

    ata_wait_bsy();
    ata_wait_drq();

    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_DATA, ptr[i]);
    }

    outb(ATA_PRIMARY_COMMAND, 0xE7); // Cache Flush
}
