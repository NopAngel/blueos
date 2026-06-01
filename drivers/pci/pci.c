#include <kernel/ports.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <lib/string.h>
#include <drivers/pci.h>
#include <drivers/disk.h>
#include <drivers/cdrom.h>
#include <mm/memory.h>
#include <mm/vmm.h>


uint16_t g_smbus_base = 0;
drm_device_t g_vga_dev;
disk_info_t system_disks[4];
int disk_count = 0;

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC



void pci_write_config(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned int val) {
    unsigned int address = (unsigned int)((((unsigned int)bus) << 16) | (((unsigned int)slot) << 11) |
              (((unsigned int)func) << 8) | (offset & 0xfc) | ((unsigned int)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, val);
}


static void set_disk_info_defaults(disk_info_t* disk) {
    memset(disk, 0, sizeof(*disk));
    strcpy(disk->transport, "UNKNOWN");
    strcpy(disk->media_type, "UNKNOWN");
    strcpy(disk->partition_style, "UNKNOWN");
    disk->partition_count = 0;
}

static const char* mbr_partition_type_name(uint8_t type) {
    switch (type) {
        case 0x07: return "NTFS";
        case 0x0B: return "FAT32";
        case 0x0C: return "FAT32LBA";
        case 0x83: return "LINUX";
        case 0x82: return "SWAP";
        case 0x0F: return "EXT";
        case 0x05: return "EXT/MBR";
        case 0xEE: return "GPT_PROT";
        default:   return "UNKNOWN";
    }
}

static void disk_parse_partitions(disk_info_t* disk) {
    if (!disk || !disk->present || strcmp(disk->transport, "IDE") != 0) {
        return;
    }

    uint8_t sector[512];
    disk_read(0, sector, 1);
    if (sector[510] != 0x55 || sector[511] != 0xAA) {
        return;
    }

    disk->partition_count = 0;
    if (sector[0x1BE + 4] == 0xEE) {
        strncpy(disk->partition_style, "GPT", sizeof(disk->partition_style) - 1);
        disk->partition_style[sizeof(disk->partition_style) - 1] = '\0';

        uint8_t header[512];
        disk_read(1, header, 1);
        if (memcmp(header, "EFI PART", 8) != 0) {
            return;
        }

        uint64_t entries_lba = *(uint64_t*)&header[72];
        uint32_t entries_count = *(uint32_t*)&header[80];
        uint32_t entry_size = *(uint32_t*)&header[84];
        if (entries_count == 0 || entry_size == 0 || entry_size > 128) {
            return;
        }

        uint8_t entries[512];
        disk_read((uint32_t)entries_lba, entries, 1);
        for (uint32_t i = 0; i < entries_count && disk->partition_count < 4; i++) {
            const uint8_t* entry = entries + i * entry_size;
            bool empty = true;
            for (int j = 0; j < 16; j++) {
                if (entry[j] != 0) {
                    empty = false;
                    break;
                }
            }
            if (empty) {
                continue;
            }

            uint32_t start = *(uint32_t*)&entry[32];
            uint32_t sectors = *(uint32_t*)&entry[40];
            if (sectors == 0) {
                continue;
            }

            partition_info_t* part = &disk->partitions[disk->partition_count];
            part->active = (entry[0] & 0x80) != 0;
            part->start_lba = start;
            part->sectors = sectors;
            strncpy(part->type, "GPT", sizeof(part->type) - 1);
            part->type[sizeof(part->type) - 1] = '\0';
            sprintf(part->name, "p%d", disk->partition_count + 1);
            disk->partition_count++;
        }
        return;
    }

    strncpy(disk->partition_style, "MBR", sizeof(disk->partition_style) - 1);
    disk->partition_style[sizeof(disk->partition_style) - 1] = '\0';

    for (int i = 0; i < 4 && disk->partition_count < 4; i++) {
        uint8_t* entry = sector + 0x1BE + i * 16;
        uint8_t type = entry[4];
        uint32_t start = *(uint32_t*)&entry[8];
        uint32_t sectors = *(uint32_t*)&entry[12];
        if (type == 0 || sectors == 0) {
            continue;
        }

        partition_info_t* part = &disk->partitions[disk->partition_count];
        part->active = entry[0] == 0x80;
        part->start_lba = start;
        part->sectors = sectors;
        strncpy(part->type, mbr_partition_type_name(type), sizeof(part->type) - 1);
        part->type[sizeof(part->type) - 1] = '\0';
        sprintf(part->name, "p%d", disk->partition_count + 1);
        disk->partition_count++;
    }
}

void register_disk(const char* name_prefix,
                   const char* transport,
                   const char* media_type,
                   uint32_t bar,
                   uint16_t vendor,
                   uint16_t device,
                   uint8_t bus,
                   uint8_t slot,
                   uint8_t func) {
    if (disk_count < 4) {
        set_disk_info_defaults(&system_disks[disk_count]);

        char prefix = (name_prefix[0] == 'S') ? 's' : name_prefix[0];
        system_disks[disk_count].name[0] = prefix;
        system_disks[disk_count].name[1] = 'd';
        system_disks[disk_count].name[2] = '0' + disk_count;
        system_disks[disk_count].name[3] = '\0';

        strncpy(system_disks[disk_count].transport, transport, sizeof(system_disks[disk_count].transport) - 1);
        strncpy(system_disks[disk_count].media_type, media_type, sizeof(system_disks[disk_count].media_type) - 1);
        system_disks[disk_count].bar = bar;
        system_disks[disk_count].present = true;
        system_disks[disk_count].vendor_id = vendor;
        system_disks[disk_count].device_id = device;
        system_disks[disk_count].bus = bus;
        system_disks[disk_count].slot = slot;
        system_disks[disk_count].func = func;

        if (strcmp(transport, "IDE") == 0) {
            if (disk_is_ssd_primary()) {
                strncpy(system_disks[disk_count].media_type, "SSD", sizeof(system_disks[disk_count].media_type) - 1);
            }
            disk_detect_partition_style(system_disks[disk_count].partition_style,
                                        sizeof(system_disks[disk_count].partition_style));
            disk_parse_partitions(&system_disks[disk_count]);
        }

        disk_count++;
    }
}

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(0xCF8, address);
    return inl(0xCFC);
}


unsigned int pci_find_lsi_scsi() {
    for (unsigned short bus = 0; bus < 256; bus++) {
        for (unsigned char slot = 0; slot < 32; slot++) {
            unsigned int data = pci_read_config(bus, slot, 0, 0x0);

            if (data != 0xFFFFFFFF) {
                unsigned short vendor = data & 0xFFFF;
                unsigned short device = (data >> 16) & 0xFFFF;

                if (vendor == 0x1000 && device == 0x0012) {
                    boot_msg("PCI", "Found LSI SCSI Controller \n", 0);

                    unsigned int bar0 = pci_read_config(bus, slot, 0, 0x10);
                    unsigned int io_port = bar0 & ~0x3;

                    unsigned int command = pci_read_config(bus, slot, 0, 0x04);
                    command |= 0x7;

                    boot_msg("PCI", "LSI BAR0 (I/O Port)\n", 0);
                    return io_port;
                }
            }
        }
    }
    return 0;
}

uint32_t vbox_io_base = 0;

void pci_check_device(uint8_t bus, uint8_t device, uint8_t func) {
    uint16_t vendor = pci_read_config(bus, device, func, 0x00);
    uint16_t device_id = pci_read_config(bus, device, func, 0x02);

    if (vendor == 0x80EE && device_id == 0xCAFE) {
        vbox_io_base = pci_read_config(bus, device, func, 0x10) & ~0x1;
        printk("Found VMMDev (VBox) at port: 0x%x\n", vbox_io_base);
    }



}

uint32_t pci_get_vbox_port() {
    return vbox_io_base;
}

uint32_t pci_find_device(uint16_t vendor, uint16_t device) {
    for (int bus = 0; bus < 256; bus++) {
        for (int slot = 0; slot < 32; slot++) {
            uint32_t res = pci_read_config(bus, slot, 0, 0);
            if ((uint16_t)res == vendor && (uint16_t)(res >> 16) == device) {

                return (bus << 16) | (slot << 8);
            }
        }
    }
    return 0xFFFFFFFF;
}

void detect_piix_pm(uint8_t bus, uint8_t dev, uint8_t func) {
    uint32_t id_reg = pci_read_config(bus, dev, func, 0x00);
    uint16_t vendor = id_reg & 0xFFFF;
    uint16_t product = (id_reg >> 16) & 0xFFFF;

    uint32_t class_reg = pci_read_config(bus, dev, func, 0x08);
    uint8_t revision = class_reg & 0xFF;

    if (vendor == 0x8086 && (product == 0x7113 || product == 0x266a)) {
        printk("piixpm0 at pci %d dev %d function %d: vendor 0x%x product 0x%x (rev. 0x%02x)\n",
                bus, dev, func, vendor, product, revision);

    }
}


void pci_scan_bus() {
    boot_msg("PCI", "Scanning devices...\n", 2);

    for (int bus = 0; bus < 256; bus++) {
        for (int slot = 0; slot < 32; slot++) {

            for (int func = 0; func < 8; func++) {
                uint32_t id_reg = pci_read_config(bus, slot, func, 0);
                if ((uint16_t)id_reg == 0xFFFF) continue;

                uint16_t vendor = id_reg & 0xFFFF;
                uint16_t device_id = (id_reg >> 16) & 0xFFFF;

                uint32_t class_reg = pci_read_config(bus, slot, func, 0x08);
                uint8_t class_code = (class_reg >> 24) & 0xFF;
                uint8_t subclass   = (class_reg >> 16) & 0xFF;
                uint8_t prog_if    = (class_reg >> 8) & 0xFF;

                if (class_code == 0x0C && subclass == 0x03) {
                    uint32_t bar0 = pci_read_config(bus, slot, func, 0x10);
                    register_disk("u", "USB", "USB", bar0, vendor, device_id, bus, slot, func);
                    printk(" +   Registered: ud%d (USB storage) at %d:%d:%d\n", disk_count-1, bus, slot, func);
                }

                if (vendor == 0x1000 && device_id == 0x0012) {
                    uint32_t bar0 = pci_read_config(bus, slot, func, 0x10);
                    register_disk("s", "SCSI", "HDD", bar0, vendor, device_id, bus, slot, func);
                    printk(" +   Registered: sd%d (SCSI) at %d:%d:%d\n", disk_count-1, bus, slot, func);
                }

                if (vendor == 0x8086 && device_id == 0x7010) {
                    if (ide_drive_present(0)) {
                        uint32_t bar4 = pci_read_config(bus, slot, func, 0x20);
                        register_disk("h", "IDE", "HDD", bar4, vendor, device_id, bus, slot, func);
                        printk(" +   Registered: hd%d (IDE) at %d:%d:%d\n", disk_count-1, bus, slot, func);
                    } else {
                        printk(" +   IDE controller found at %d:%d:%d but no primary drive attached\n", bus, slot, func);
                    }
                }

                detect_piix_pm(bus, slot, func);
            }
        }
    }
}
void drm_init() {
    for(int bus = 0; bus < 256; bus++) {
        for(int slot = 0; slot < 32; slot++) {
            uint32_t device = pci_read_config(bus, slot, 0, 0x08);
            uint8_t class_code = (device >> 24) & 0xFF;

            if (class_code == 0x03) {
               // printk("<6> [DRM] GPU detected on Bus %d, Slot %d\n", bus, slot);
               // setup_vga_driver(bus, slot); // inited (??)
            }
        }
    }
}


