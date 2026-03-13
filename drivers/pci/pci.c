#include <blueos/ports.h>
#include <blueos/printk.h>
#include <blueos/colors.h>


#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

/**
 * pci_read_config - Lee un registro de 32 bits del espacio PCI
 */
void pci_write_config(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned int val) {
    unsigned int address = (unsigned int)((((unsigned int)bus) << 16) | (((unsigned int)slot) << 11) |
              (((unsigned int)func) << 8) | (offset & 0xfc) | ((unsigned int)0x80000000));
    
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, val);
}

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) | (offset & 0xfc) | ((uint32_t)0x80000000);
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
                    printk(GREEN, "[ PCI ] Found LSI SCSI Controller at %d:%d:0\n", bus, slot);
                    
                    unsigned int bar0 = pci_read_config(bus, slot, 0, 0x10);
                    unsigned int io_port = bar0 & ~0x3; 

                    unsigned int command = pci_read_config(bus, slot, 0, 0x04);
                    command |= 0x7; 
                    
                    printk(CYAN, "[ PCI ] LSI BAR0 (I/O Port): 0x%x\n", io_port);
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
        printk(CYAN, "Found VMMDev (VBox) at port: 0x%x\n", vbox_io_base);
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