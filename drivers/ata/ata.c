#include <kernel/io.h> 


#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERR          0x1F1
#define ATA_PRIMARY_SECCOUNT     0x1F2
#define ATA_PRIMARY_LBA_LO       0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HI       0x1F5
#define ATA_PRIMARY_DRIVE_SEL    0x1F6
#define ATA_PRIMARY_COMM_STAT    0x1F7

void ata_wait_ready() {
    while ((inb(ATA_PRIMARY_COMM_STAT) & 0x80)); 
    while (!(inb(ATA_PRIMARY_COMM_STAT) & 0x40)); 
}

void ata_read_sectors(uint32_t lba, uint8_t count, uint8_t *buf) {
    ata_wait_ready();
    outb(ATA_PRIMARY_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, count);
    outb(ATA_PRIMARY_LBA_LO, (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HI, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMM_STAT, 0x20); // Read Sectors

    uint16_t *ptr = (uint16_t *)buf;
    for (int i = 0; i < count; i++) {
        ata_wait_ready();
        for (int j = 0; j < 256; j++) {
            *ptr++ = inw(ATA_PRIMARY_DATA);
        }
    }
}

void ata_write_sectors(uint32_t lba, uint8_t count, uint8_t *buf) {
    ata_wait_ready();
    outb(ATA_PRIMARY_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, count);
    outb(ATA_PRIMARY_LBA_LO, (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HI, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMM_STAT, 0x30); //  Write Sectors

    uint16_t *ptr = (uint16_t *)buf;
    for (int i = 0; i < count; i++) {
        ata_wait_ready();
        for (int j = 0; j < 256; j++) {
            outw(ATA_PRIMARY_DATA, *ptr++);
        }
    }
}
void ata_read_sector(uint32_t lba, uint8_t *buf) {
    // 1. Esperar a que el disco esté listo (BSY=0)
    while (inb(0x1F7) & 0x80);

    // 2. Configurar registros LBA
    outb(0x1F2, 1); // Queremos 1 sector
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F7, 0x20); // Comando: Read Sectors

    // 3. Esperar a que los datos estén listos para ser recogidos (DRQ=1)
    while (!(inb(0x1F7) & 0x08));

    // 4. LEER MANUALMENTE (Sustituto de insw)
    // Convertimos el buffer de bytes a un puntero de 16 bits (words)
    uint16_t *ptr = (uint16_t *)buf;
    for (int i = 0; i < 256; i++) { // 256 words = 512 bytes
        ptr[i] = inw(0x1F0); // Leemos de 2 en 2 bytes desde el puerto de datos
    }
}