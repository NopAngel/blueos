#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_TTYS 4
#define VIDEO_MEM 0xB8000
#define TTY_WIDTH 80
#define TTY_HEIGHT 25

typedef struct {
  int cursor_x;
  int cursor_y;
  uint16_t screen_buffer[TTY_WIDTH * TTY_HEIGHT];
} tty_t;

void _xterm_tty_init(void);
void _xterm_tty_switch(int id);
void _xterm_tty_write(const char *str);

#endif
