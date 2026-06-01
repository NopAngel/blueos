#include <drivers/usb.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

void usbscan_init() {
    printk("USB", "Scanning UHCI controller\n", 0);

    for (int port = 0; port < 2; port++) {
        uint16_t port_reg = USB_BASE_UHCI + 0x10 + (port * 2);
        uint16_t status = usb_in16(port_reg);

        if (status & 0x01) { /* Device connected */
            printk("  -> Port %d: Device found! Resetting...\n", port);

            /* Port Reset Sequence */
            usb_out16(port_reg, status | 0x0200); /* Write Reset bit */
            for(volatile int i = 0; i < 2000000; i++); /* Wait for device */
            usb_out16(port_reg, status & ~0x0200); /* Clear Reset */

            /* Enable Port */
            usb_out16(port_reg, usb_in16(port_reg) | 0x0004);
        }
    }
}

/**
 * usb_poll_interrupt - Generic interface to read HID reports
 * For now, it's a stub that keeps the service alive.
 */
int usb_poll_interrupt(uint8_t endpoint, uint8_t *buffer, uint32_t len) {
    /* To make this functional, we'll need to implement
       Transfer Descriptors (TDs) and Queue Heads (QHs) later. */

    uint16_t status = usb_in16(USB_BASE_UHCI + 0x10);
    if (!(status & 0x01)) return 0;

    return 0; /* Returning 0 bytes so it doesn't crash the DS3 service */
}
