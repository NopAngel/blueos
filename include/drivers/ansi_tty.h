#ifndef _ANSI_TTY_H
#define _ANSI_TTY_H

#include <stdint.h>

void ansi_tty_init();
void ansi_tty_putc(char c);
void ansi_tty_puts(const char *s);

#endif