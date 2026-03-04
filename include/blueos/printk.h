#ifndef PRINTK_H
#define PRINTK_H
#include <stdarg.h>


unsigned int printk(unsigned int color, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
unsigned int vprintk(unsigned int color, const char *fmt, va_list args);
void clear_screen();

#endif