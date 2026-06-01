#ifndef _DRIVERS_SERIAL_H
#define _DRIVERS_SERIAL_H

#include <stdint.h>

#define COM1 0x3F8

void serial_init(void);
static int is_transmit_empty(void);
void serial_putc(char c);
void serial_puts(const char* str);


#endif
