#ifndef BLUEOS_KEYBOARD_H
#define BLUEOS_KEYBOARD_H

#include <stdint.h>

#define INPUT_BUFFER_SIZE 256
#define HISTORY_MAX       10

#define KBD_F1 0x3B
#define KBD_F2 0x3C
#define KBD_F3 0x3D
#define KBD_F4 0x3E
#define KBD_F5 0x3F
#define KBD_F6 0x40
#define KBD_F7 0x41

/* Shared State */
extern char input_buffer[INPUT_BUFFER_SIZE];
extern int input_index;
extern int keyboard_echo;

/* Main Logic */
void keyboard_init(void);
void keyboard_handler();
void handle_backspace(void);

/* Arch-Specific Bridge (Must be implemented in arch/xxx/keyboard_io.c) */
uint8_t arch_get_scancode(void);
void arch_put_char(char c, unsigned int color);

#endif
