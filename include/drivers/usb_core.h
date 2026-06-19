#ifndef __USB_CORE_H
#define __USB_CORE_H

int usb_core_register_device(uint16_t vid, uint16_t did, uint8_t speed);
void usb_core_init(void);

#endif
