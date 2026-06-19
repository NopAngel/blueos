#ifndef USB_H
#define USB_H

#include <stdbool.h>
#include <stdint.h>

#define DS3_ENDPOINT 0x81
#define USB_BASE_UHCI 0xD000

/* Multi-arch I/O abstraction */
#ifdef __i386__
#include <kernel/io.h>
#define usb_out16(port, data) outw(port, data)
#define usb_in16(port) inw(port)
#else
/* MMIO for RISC-V or ARM */
#define usb_out16(addr, data) (*(volatile uint16_t *)(addr) = data)
#define usb_in16(addr) (*(volatile uint16_t *)(addr))
#endif

void usbscan_init(void);
int usb_poll_interrupt(uint8_t endpoint, uint8_t *buffer, uint32_t len);

#endif