#ifndef KEYBOARD_H
#define KEYBOARD_H

unsigned char read_scancode();

void scroll_screen();

void put_char(char c);
int process_input ();
void keyboard_handler();

#endif
