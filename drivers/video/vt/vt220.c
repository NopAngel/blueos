#include <drivers/vt220.h>
#include <kernel/ports.h>

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

static int x = 0;
static int y = 0;
static unsigned char color = 0x0F;

static void set_cursor(int x, int y) {
  unsigned short pos = y * MAX_COLS + x;

  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

static void scroll(void) {
  unsigned short *video_memory = (unsigned short *)VIDEO_ADDRESS;
  int i;

  for (i = 0; i < (MAX_ROWS - 1) * MAX_COLS; i++) {
    video_memory[i] = video_memory[i + MAX_COLS];
  }

  for (i = (MAX_ROWS - 1) * MAX_COLS; i < MAX_ROWS * MAX_COLS; i++) {
    video_memory[i] = ' ' | (color << 8);
  }

  y = MAX_ROWS - 1;
}

void vt220_init(void) {
  unsigned short *video_memory = (unsigned short *)VIDEO_ADDRESS;
  int i;

  for (i = 0; i < MAX_ROWS * MAX_COLS; i++) {
    video_memory[i] = ' ' | (color << 8);
  }

  x = 0;
  y = 0;
  set_cursor(x, y);
}

static void vt220_set_color(unsigned char c) { color = c; }

void vt220_putc(char c) {
  unsigned short *video_memory = (unsigned short *)VIDEO_ADDRESS;

  if (c == '\n') {
    x = 0;
    y++;
  } else if (c == '\r') {
    x = 0;
  } else if (c == '\b') {
    if (x > 0) {
      x--;
    }
  } else if (c == '\x1b') {
    // ANSI escape code
    char next = *(&c + 1);
    if (next == '[') {
      char code = *(&c + 2);
      char m = *(&c + 3);
      if (m == 'm') {
        if (code == '0') {
          vt220_set_color(0x0F);
        } else if (code == '3') {
          char color_code = *(&c + 4);
          if (color_code == '0') {
            vt220_set_color(0x00);
          } else if (color_code == '1') {
            vt220_set_color(0x04);
          } else if (color_code == '2') {
            vt220_set_color(0x02);
          } else if (color_code == '3') {
            vt220_set_color(0x06);
          } else if (color_code == '4') {
            vt220_set_color(0x01);
          } else if (color_code == '5') {
            vt220_set_color(0x05);
          } else if (color_code == '6') {
            vt220_set_color(0x03);
          } else if (color_code == '7') {
            vt220_set_color(0x07);
          }
        }
      }
    }
  } else {
    video_memory[y * MAX_COLS + x] = c | (color << 8);
    x++;
  }

  if (x >= MAX_COLS) {
    x = 0;
    y++;
  }

  if (y >= MAX_ROWS) {
    scroll();
  }

  set_cursor(x, y);
}

void vt220_puts(const char *s) {
  while (*s) {
    vt220_putc(*s++);
  }
}
