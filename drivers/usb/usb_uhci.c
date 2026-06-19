#include <kernel/colors.h>
#include <kernel/ports.h>
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "USB_UHCI"

/* UHCI I/O Register Space Offsets (Legacy Intel PiIX4 / ICH specs) */
#define UHCI_REG_USBCMD 0x00    /* USB Command Register */
#define UHCI_REG_USBSTS 0x02    /* USB Status Register */
#define UHCI_REG_USBINTR 0x04   /* USB Interrupt Enable */
#define UHCI_REG_FRNUM 0x06     /* Frame Number Register */
#define UHCI_REG_FLBASEADD 0x08 /* Frame List Base Address */
#define UHCI_REG_SOFMOD 0x0C    /* Start of Frame Modify */
#define UHCI_REG_PORTSC1 0x10   /* Port 1 Status and Control */
#define UHCI_REG_PORTSC2 0x12   /* Port 2 Status and Control */

static uint32_t g_uhci_io_base = 0;

extern void usb_hub_port_status_changed(int port_id, int connected);

/**
 * usb_uhci_init: Initialized by the PCI scan loop if an Intel UHCI controller
 * signature matches.
 */
int usb_uhci_init(uint32_t io_base) {
  g_uhci_io_base = io_base;

  boot_msg(MODULE_NAME, "Probing Intel UHCI Host Controller...", 0);
  printk("<6>[  %s  ] Mapping legacy controller I/O base ports at: 0x%04X\n",
         MODULE_NAME, io_base);

  /* Issue a Global Reset command via I/O port outw */
  outw(g_uhci_io_base + UHCI_REG_USBCMD, 0x0004);
  for (volatile int i = 0; i < 50000; i++)
    ; /* Hardware settling delay step */
  outw(g_uhci_io_base + UHCI_REG_USBCMD, 0x0000);

  /* Turn off legacy interrupts, let BlueOS handle it clean */
  outw(g_uhci_io_base + UHCI_REG_USBINTR, 0x0000);

  /* Read Port Status to check for pre-connected devices during boot */
  uint16_t port1 = inw(g_uhci_io_base + UHCI_REG_PORTSC1);
  boot_msg(MODULE_NAME, "UHCI controller logic out of reset state.", 0);

  if (port1 & (1 << 0)) { /* Bit 0: Current Connect Status */
    /* Notify the hub layer that a device is present on boot */
    usb_hub_port_status_changed(1, 1);
  }

  return 0;
}