#ifndef _VT100_H
#define _VT100_H

#include <stdint.h>

void vt100_init(void);
void vt100_putc(char c);
void vt100_puts(const char *s);
void vt100_set_color(uint8_t color);

#endif