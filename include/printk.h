#ifndef PRINTK_H
#define PRINTK_H

unsigned int printk(unsigned int color, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

void clear_screen();

#endif