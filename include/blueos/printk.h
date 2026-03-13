#ifndef PRINTK_H
#define PRINTK_H
#include <stdarg.h>
#include <blueos/colors.h>


#define KERN_SOH "\001"        
#define LOG_BUFFER_SIZE 4096
#define KERN_EMERG   KERN_SOH "0"
#define KERN_ERROR   KERN_SOH "3"
#define KERN_WARNING KERN_SOH "4"
#define KERN_INFO    KERN_SOH "6"
#define KERN_DEBUG   KERN_SOH "7"

#define DEFAULT_MESSAGE_LOGLEVEL 4

#define pr_emerg(fmt, ...)   printk(RED,    KERN_EMERG fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)     printk(RED,    KERN_ERROR fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...)    printk(YELLOW, KERN_WARNING fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...) printk(WHITE, KERN_INFO fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...)   printk(GRAY,   KERN_DEBUG fmt, ##__VA_ARGS__)


unsigned int printk(unsigned int color, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
unsigned int vprintk(unsigned int color, const char *fmt, va_list args);
void clear_screen();

#endif