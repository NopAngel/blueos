#ifndef PCI_H
#define PCI_H


void pci_write_config(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned int val);

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
unsigned int pci_find_lsi_scsi();

void pci_check_device(uint8_t bus, uint8_t device, uint8_t func);
uint32_t pci_get_vbox_port();

uint32_t pci_find_device(uint16_t vendor, uint16_t device);
void pci_scan_bus();

#endif
