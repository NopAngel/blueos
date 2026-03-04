#include <drivers/keyboard.h>
#include <blueos/printk.h>
#include <blueos/colors.h>
#include <blueos/ports.h>
#include <lib/string.h>

#define KEYBOARD_PORT 0x60
#define SCREEN_BUFFER ((unsigned char *)0xb8000)
#define SCREEN_COLUMNS 80
#define SCREEN_ROWS 25
#define INPUT_BUFFER_SIZE 255

char input_buffer[INPUT_BUFFER_SIZE];
int input_index = 0;
int caps_lock = 0;
int shift_pressed = 0;
int ctrl_pressed = 0;
volatile unsigned char last_scancode = 0;

int keyboard_echo = 1;

int cursor_x = 0;
int cursor_y = 0;

extern void execute_shell_command(char* input);
extern char scancode_to_ascii[]; 

char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 
    0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // f1-f12
};

void set_keyboard_echo(int on) {
    keyboard_echo = 1;
}

void update_cursor(int x, int y) {
    uint16_t pos = y * SCREEN_COLUMNS + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void scroll_screen() {
    for (int i = 1; i < SCREEN_ROWS; i++) {
        memcpy(SCREEN_BUFFER + (i-1) * SCREEN_COLUMNS * 2, SCREEN_BUFFER + i * SCREEN_COLUMNS * 2, SCREEN_COLUMNS * 2);
    }
    uint16_t* last_line = (uint16_t*)(SCREEN_BUFFER + (SCREEN_ROWS - 1) * SCREEN_COLUMNS * 2);
    for (int j = 0; j < SCREEN_COLUMNS; j++) last_line[j] = 0x0720; 
    cursor_y = SCREEN_ROWS - 1; 
}

void put_char(char c, unsigned int color) {
    if (cursor_y >= SCREEN_ROWS) scroll_screen();

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        int pos = (cursor_y * SCREEN_COLUMNS + cursor_x) * 2;
        SCREEN_BUFFER[pos] = c;
        SCREEN_BUFFER[pos + 1] = (unsigned char)color;
        cursor_x++;
        if (cursor_x >= SCREEN_COLUMNS) { cursor_x = 0; cursor_y++; }
    }
    if (cursor_y >= SCREEN_ROWS) scroll_screen();
    update_cursor(cursor_x, cursor_y); 
}

void handle_backspace() {
    if (input_index > 0) {
        input_index--;
        if (cursor_x > 0) cursor_x--;
        else if (cursor_y > 0) { cursor_y--; cursor_x = SCREEN_COLUMNS - 1; }
        
        int pos = (cursor_y * SCREEN_COLUMNS + cursor_x) * 2;
        SCREEN_BUFFER[pos] = ' '; SCREEN_BUFFER[pos + 1] = 0x07;
        update_cursor(cursor_x, cursor_y);
    }
}


void keyboard_handler(struct registers *r) {
    unsigned char scancode = inb(KEYBOARD_PORT);
    pic_send_eoi(1); 

    if (scancode & 0x80) 
    {
        unsigned char released = scancode & 0x7F; 
        if (released == 0x1D) ctrl_pressed = 0;
        if (released == 0x2A || released == 0x36) shift_pressed = 0;
        last_scancode = 0;
        return;
    }

    if (scancode == last_scancode) return;
    last_scancode = scancode;

    switch (scancode) {
        case 0x1D: ctrl_pressed = 1; return;
        case 0x2A: case 0x36: shift_pressed = 1; return;
        case 0x3A: caps_lock = !caps_lock; return;
        case 0x48: /* top */ return;
        case 0x50: /* down */ return;
    }

    if (scancode < 128) {
        char ascii = scancode_to_ascii[scancode];
        if (!ascii) return;

        if ((caps_lock || shift_pressed) && ascii >= 'a' && ascii <= 'z') ascii -= 32;

        // Ctrl + L (clear_screen)
        if (ctrl_pressed && ascii == 'l') {
            clear_screen(); cursor_x = 0; cursor_y = 0;
            input_index = 0; update_cursor(0, 0);
            return;
        }


        if (ascii == '\b') {
            handle_backspace();
        } else if (ascii == '\n') {
            put_char('\n', WHITE);
            input_buffer[input_index] = '\0';
            
            execute_shell_command(input_buffer);
            
            input_index = 0;
        } else if (input_index < INPUT_BUFFER_SIZE - 1) {
            input_buffer[input_index++] = ascii;
            if (keyboard_echo) {
                put_char(ascii, WHITE);
            }
        }
    }
}