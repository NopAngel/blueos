//#include <stdint.h>
#include <kernel/io.h>
#include <kernel/printk.h>
#include <kernel/colors.h>


#define USB_BASE_ADDR 0xD000 

void usbscan_init() {
    printk(GREEN, "USB: Scanning USB ports (UHCI)...\n");

    for (int port = 0; port < 2; port++) {
        uint16_t port_reg = USB_BASE_ADDR + 0x10 + (port * 2);
        uint16_t status = inw(port_reg);

        if (status & 0x01) {
            printk(WHITE, " USB: Device detected in port %d!\n", port);

            outw(port_reg, status | 0x02); 
        }
    }
}

