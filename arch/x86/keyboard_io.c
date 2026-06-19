#include <drivers/keyboard.h>
#include <kernel/ports.h>
#include <stdint.h>

/* --- VGA Text Mode Constants --- */
#define VIDEO_ADDRESS 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x07

extern int cursor_x;
extern int cursor_y;
/**
 * scroll_screen: Desplaza todo el contenido una línea hacia arriba
 */
static void scroll_screen() {
  uint16_t *video_mem = (uint16_t *)VIDEO_ADDRESS;

  for (int i = 0; i < (MAX_ROWS - 1) * MAX_COLS; i++) {
    video_mem[i] = video_mem[i + MAX_COLS];
  }

  for (int i = (MAX_ROWS - 1) * MAX_COLS; i < MAX_ROWS * MAX_COLS; i++) {
    video_mem[i] = (WHITE_ON_BLACK << 8) | ' ';
  }

  cursor_y = MAX_ROWS - 1;
}

/**
 * update_cursor
 */
void update_cursor() {
  uint16_t pos = cursor_y * MAX_COLS + cursor_x;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (uint8_t)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/**
 * arch_get_scancode
 */
uint8_t arch_get_scancode() {
  if (inb(0x64) & 1) {
    return inb(0x60);
  }
  return 0;
}

/**
 * arch_put_char
 */
void arch_put_char(char c, unsigned int color) {
  uint16_t *video_mem = (uint16_t *)VIDEO_ADDRESS;
  uint8_t attribute = (uint8_t)color;

  if (c == '\n') {
    cursor_x = 0;
    cursor_y++;
  } else if (c == '\b') {
    if (cursor_x > 0) {
      cursor_x--;
    } else if (cursor_y > 0) {
      cursor_y--;
      cursor_x = MAX_COLS - 1;
    }
    video_mem[cursor_y * MAX_COLS + cursor_x] = (attribute << 8) | ' ';
  } else {
    video_mem[cursor_y * MAX_COLS + cursor_x] = (attribute << 8) | c;
    cursor_x++;
  }

  if (cursor_x >= MAX_COLS) {
    cursor_x = 0;
    cursor_y++;
  }

  if (cursor_y >= MAX_ROWS) {
    scroll_screen();
  }

  update_cursor();
}
