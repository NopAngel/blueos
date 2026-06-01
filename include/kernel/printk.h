#ifndef BLUEOS_PRINTK_H
#define BLUEOS_PRINTK_H

#include <kernel/colors.h>
#include <stdarg.h>
#include <stdint.h>

/* Log Buffer Configuration */
#define LOG_BUFFER_SIZE 4096

/* 2. Printk prototypes */
unsigned int printk(const char *fmt, ...);
unsigned int vprintk(const char *fmt, va_list args);
void clear_screen(void);
void putchar(char c, unsigned int color);
int sprintf(char *buf, const char *fmt, ...);
void boot_msg(const char *subsystem, const char *msg, int status);

#endif
