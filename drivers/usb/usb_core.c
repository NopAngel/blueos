#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "USB_CORE"
#define MAX_USB_DEVICES 127

typedef struct {
  uint8_t address;
  uint8_t speed;
  uint16_t vendor_id;
  uint16_t device_id;
  int is_active;
} usb_device_t;

static usb_device_t g_usb_dev_table[MAX_USB_DEVICES];
static uint8_t g_next_usb_address = 1;

/**
 * usb_core_register_device: Assigns a unique bus address and enumerates a new
 * USB node.
 */
int usb_core_register_device(uint16_t vid, uint16_t did, uint8_t speed) {
  if (g_next_usb_address >= MAX_USB_DEVICES) {
    printk("<3>[  %s  ] Error: Maximum USB address limit reached on the core "
           "bus.\n",
           MODULE_NAME);
    return -1;
  }

  uint8_t addr = g_next_usb_address++;
  g_usb_dev_table[addr].address = addr;
  g_usb_dev_table[addr].vendor_id = vid;
  g_usb_dev_table[addr].device_id = did;
  g_usb_dev_table[addr].speed = speed;
  g_usb_dev_table[addr].is_active = 1;

  printk("<6>[  %s  ] New device enumerated -> Addr: %d, VID: 0x%04X, PID: "
         "0x%04X, Speed Mode: %d\n",
         MODULE_NAME, addr, vid, did, speed);

  return (int)addr;
}

/**
 * usb_core_init: Core subsystem bootstrap routine.
 */
void usb_core_init(void) {
  boot_msg(MODULE_NAME, "Initializing Universal Serial Bus Core Stack...", 0);

  /* Clear device registry table */
  for (int i = 0; i < MAX_USB_DEVICES; i++) {
    g_usb_dev_table[i].is_active = 0;
  }

  boot_msg(MODULE_NAME, "USB request block (URB) routing engine ready.", 0);
}