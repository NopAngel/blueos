#include <kernel/types.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

extern void pci_check_device(uint8_t bus, uint8_t device, uint8_t func);

void find_wifi_card() {
    // 0x8086 = Intel
    uint32_t device = pci_find_device(0x8086, 0x4242); 
    if (device != -1) {
        printk("BlueOS: Tarjeta Intel Wireless detectada!\n");
    }
}