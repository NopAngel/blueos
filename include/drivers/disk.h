#ifndef DISK_H
#define DISK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Firmas que espera tu EXT2
void disk_read(uint32_t lba, uint8_t *buffer, uint32_t sectors);
void disk_write(uint32_t lba, uint8_t *buffer, uint32_t sectors);

bool ide_identify_drive(uint8_t slave, uint16_t *buffer);
bool ide_drive_present(uint8_t slave);
bool disk_is_ssd_primary(void);
void disk_detect_partition_style(char *out_style, size_t out_size);

#endif
