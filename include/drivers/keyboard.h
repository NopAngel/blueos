#ifndef _DRIVERS_KEYBOARD_H
#define _DRIVERS_KEYBOARD_H

#define KEYBOARD_PORT 0x60
#define SCREEN_BUFFER ((unsigned char *)0xb8000)
#define HISTORY_MAX 10
#define SCREEN_COLUMNS 80
#define SCREEN_ROWS 25
#define INPUT_BUFFER_SIZE 255

void put_char(char c, unsigned int color);
void scroll_screen();
void keyboard_handler();

#endif