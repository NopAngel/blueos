#ifndef BLUEOS_KEYBOARD_H
#define BLUEOS_KEYBOARD_H

#include <stdint.h>

#define INPUT_BUFFER_SIZE 256
#define HISTORY_MAX       10

/* Shared State */
extern char input_buffer[INPUT_BUFFER_SIZE];
extern int input_index;
extern int keyboard_echo;

/* Main Logic */
void keyboard_init(void);
void keyboard_handler(void);
void handle_backspace(void);

/* Arch-Specific Bridge (Must be implemented in arch/xxx/keyboard_io.c) */
uint8_t arch_get_scancode(void);
void arch_put_char(char c, unsigned int color);

#endif