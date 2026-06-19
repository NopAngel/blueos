#include <drivers/disk.h>
#include <kernel/ports.h> // Necesitas inb/outb/outw
#include <kernel/types.h>
#include <lib/string.h>
#include <stdbool.h>

#define IDE_PRIMARY_DATA 0x1F0
#define IDE_PRIMARY_ERR 0x1F1
#define IDE_PRIMARY_SECCOUNT 0x1F2
#define IDE_PRIMARY_LBA_LO 0x1F3
#define IDE_PRIMARY_LBA_MID 0x1F4
#define IDE_PRIMARY_LBA_HI 0x1F5
#define IDE_PRIMARY_DRIVE_SEL 0x1F6
#define IDE_PRIMARY_COMM_STAT 0x1F7

#define IDE_TIMEOUT 100000

static bool ide_wait_bsy_timeout(void) {
  int timeout = IDE_TIMEOUT;
  while ((inb(IDE_PRIMARY_COMM_STAT) & 0x80) != 0) {
    if (--timeout <= 0)
      return false;
  }
  return true;
}

static bool ide_wait_drq_timeout(void) {
  int timeout = IDE_TIMEOUT;
  while ((inb(IDE_PRIMARY_COMM_STAT) & 0x08) == 0) {
    if (--timeout <= 0)
      return false;
  }
  return true;
}

static void ide_select_drive(uint8_t slave) {
  uint8_t sel = 0xA0 | (slave ? 0x10 : 0x00);
  outb(IDE_PRIMARY_DRIVE_SEL, sel);
  ide_wait_bsy_timeout();
}

bool ide_identify_drive(uint8_t slave, uint16_t *buffer) {
  if (buffer == NULL)
    return false;

  ide_select_drive(slave);

  outb(IDE_PRIMARY_SECCOUNT, 0);
  outb(IDE_PRIMARY_LBA_LO, 0);
  outb(IDE_PRIMARY_LBA_MID, 0);
  outb(IDE_PRIMARY_LBA_HI, 0);
  outb(IDE_PRIMARY_COMM_STAT, 0xEC);

  uint8_t status = inb(IDE_PRIMARY_COMM_STAT);
  if (status == 0)
    return false;

  if (!ide_wait_bsy_timeout())
    return false;

  status = inb(IDE_PRIMARY_COMM_STAT);
  if (status & 0x01)
    return false;

  if (!ide_wait_drq_timeout())
    return false;

  for (int i = 0; i < 256; i++) {
    buffer[i] = inw(IDE_PRIMARY_DATA);
  }

  return true;
}

bool ide_drive_present(uint8_t slave) {
  uint16_t identify[256];
  return ide_identify_drive(slave, identify);
}

bool disk_is_ssd_primary(void) {
  uint16_t identify[256];
  if (!ide_identify_drive(0, identify)) {
    return false;
  }
  return identify[217] == 0xFFFF;
}

void disk_detect_partition_style(char *out_style, size_t out_size) {
  if (out_style == NULL || out_size == 0)
    return;

  uint16_t identify[256];
  if (!ide_identify_drive(0, identify)) {
    strncpy(out_style, "UNKNOWN", out_size - 1);
    out_style[out_size - 1] = '\0';
    return;
  }

  uint8_t sector[512];
  disk_read(0, sector, 1);

  if (sector[510] != 0x55 || sector[511] != 0xAA) {
    strncpy(out_style, "UNKNOWN", out_size - 1);
    out_style[out_size - 1] = '\0';
    return;
  }

  if (sector[0x1BE + 4] == 0xEE) {
    strncpy(out_style, "GPT", out_size - 1);
  } else {
    strncpy(out_style, "MBR", out_size - 1);
  }
  out_style[out_size - 1] = '\0';
}

void disk_read(uint32_t lba, uint8_t *buffer, uint32_t sectors) {
  outb(IDE_PRIMARY_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
  outb(IDE_PRIMARY_SECCOUNT, (uint8_t)sectors);
  outb(IDE_PRIMARY_LBA_LO, (uint8_t)lba);
  outb(IDE_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
  outb(IDE_PRIMARY_LBA_HI, (uint8_t)(lba >> 16));
  outb(IDE_PRIMARY_COMM_STAT, 0x20); // Comando: Identificar/Leer sectores

  uint16_t *ptr = (uint16_t *)buffer;
  for (uint32_t i = 0; i < sectors; i++) {
    if (!ide_wait_bsy_timeout() || !ide_wait_drq_timeout())
      return;
    for (int j = 0; j < 256; j++) {
      *ptr++ = inw(IDE_PRIMARY_DATA);
    }
  }
}

void disk_write(uint32_t lba, uint8_t *buffer, uint32_t sectors) {
  outb(IDE_PRIMARY_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
  outb(IDE_PRIMARY_SECCOUNT, (uint8_t)sectors);
  outb(IDE_PRIMARY_LBA_LO, (uint8_t)lba);
  outb(IDE_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
  outb(IDE_PRIMARY_LBA_HI, (uint8_t)(lba >> 16));
  outb(IDE_PRIMARY_COMM_STAT, 0x30); // Comando: Escribir sectores

  uint16_t *ptr = (uint16_t *)buffer;
  for (uint32_t i = 0; i < sectors; i++) {
    if (!ide_wait_bsy_timeout() || !ide_wait_drq_timeout())
      return;
    for (int j = 0; j < 256; j++) {
      outw(IDE_PRIMARY_DATA, *ptr++);
    }
  }
}
