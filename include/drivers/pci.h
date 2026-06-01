#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t* physical_base;
    uint32_t* virtual_base;
    uint32_t  width;
    uint32_t  height;
    uint32_t  pitch;
    uint8_t   bpp;
} drm_device_t;

typedef struct partition_info_t {
    char     name[16];
    uint32_t start_lba;
    uint32_t sectors;
    char     type[16];
    bool     active;
} partition_info_t;

typedef struct disk_info_t {
    char             name[16];
    uint32_t         bar;
    bool             present;
    char             transport[16];
    char             media_type[16];
    char             partition_style[16];
    uint16_t         vendor_id;
    uint16_t         device_id;
    uint8_t          bus;
    uint8_t          slot;
    uint8_t          func;
    uint8_t          partition_count;
    partition_info_t partitions[4];
} disk_info_t;

extern disk_info_t system_disks[4];
extern int disk_count;

extern drm_device_t g_vga_dev;

void pci_write_config(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned int val);

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
unsigned int pci_find_lsi_scsi();

void pci_check_device(uint8_t bus, uint8_t device, uint8_t func);
uint32_t pci_get_vbox_port();

uint32_t pci_find_device(uint16_t vendor, uint16_t device);
void pci_scan_bus();
void drm_init();
void setup_vga_driver(uint8_t bus, uint8_t slot);

#endif
