#ifndef ATA_H
#define ATA_H

#include <stdint.h>

/* --- Puertos I/O para el bus Primario --- */
#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERR          0x1F1
#define ATA_PRIMARY_SECCOUNT     0x1F2
#define ATA_PRIMARY_LBA_LOW      0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HIGH     0x1F5
#define ATA_PRIMARY_DRIVE_SEL    0x1F6
#define ATA_PRIMARY_COMMAND      0x1F7
#define ATA_PRIMARY_STATUS       0x1F7

/* --- Comandos ATA --- */
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_IDENTIFY          0xEC

/* --- Bits de Status --- */
#define ATA_STATUS_BSY            0x80  // Busy
#define ATA_STATUS_DRDY           0x40  // Drive Ready
#define ATA_STATUS_DF             0x20  // Drive Fault
#define ATA_STATUS_DRQ            0x08  // Data Request (listo para transferir)
#define ATA_STATUS_ERR            0x01  // Error

/**
 * Lee sectores del disco usando LBA de 28 bits.
 */
void ata_read_sector(uint32_t lba, uint8_t* buffer);

/**
 * Escribe sectores al disco.
 */
void ata_write_sector(uint32_t lba, uint8_t* buffer);

#endif
