#ifndef BLUEOS_PRINTK_H
#define BLUEOS_PRINTK_H

#include <stdint.h>
#include <stdarg.h>
#include <kernel/colors.h>

/* 1. Log Levels Definitions */
#define KERN_EMERG   "[EMERG] "
#define KERN_ERR     "[ERROR] "
#define KERN_WARN    "[WARN ] " 
#define KERN_INFO    "[INFO ] "
#define KERN_DEBUG   "[DEBUG] "

/* Log Buffer Configuration */
#define LOG_BUFFER_SIZE 4096

/* 2. Printk prototypes */
unsigned int printk(unsigned int color, const char *fmt, ...);
unsigned int vprintk(unsigned int color, const char *fmt, va_list args);
void clear_screen(void);
void putchar(char c, unsigned int color);

#define pr_info(fmt, ...) \
    printk(WHITE, KERN_INFO fmt, ##__VA_ARGS__)

#define pr_warn(fmt, ...) \
    printk(YELLOW, KERN_WARN fmt, ##__VA_ARGS__)

#define pr_err(fmt, ...) \
    printk(RED, KERN_ERR fmt, ##__VA_ARGS__)

#define pr_debug(fmt, ...) \
    printk(CYAN, KERN_DEBUG fmt, ##__VA_ARGS__)

#define pr_emerg(fmt, ...) \
    printk(RED, KERN_EMERG fmt, ##__VA_ARGS__)

#endif