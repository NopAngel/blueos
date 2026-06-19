#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "USB_HUB"

extern int usb_core_register_device(uint16_t vid, uint16_t did, uint8_t speed);

/**
 * usb_hub_port_status_changed: Event handler triggered when a physical
 * electrical change occurs on a port.
 */
void usb_hub_port_status_changed(int port_id, int connected) {
  printk("<5>[  %s  ] Event detected on Root Hub Port %d\n", MODULE_NAME,
         port_id);

  if (connected) {
    boot_msg(MODULE_NAME, "Device attached! Initiating bus reset sequence...",
             0);

    /* Simulated extraction of device descriptors (e.g., a vintage USB Mouse) */
    uint16_t mock_vid = 0x046D; /* Logitech Vendor Signature */
    uint16_t mock_pid = 0xC00E; /* Classic Wheel Mouse ID */

    int assigned_addr =
        usb_core_register_device(mock_vid, mock_pid, 1); /* 1 = Full Speed */
    if (assigned_addr > 0) {
      printk("<6>[  %s  ] Hub bound port %d successfully to address %d.\n",
             MODULE_NAME, port_id, assigned_addr);
    }
  } else {
    printk("<4>[  %s  ] Device detached from Root Hub Port %d. Reclaiming bus "
           "structures.\n",
           MODULE_NAME, port_id);
  }
}