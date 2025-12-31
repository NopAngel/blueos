// keyboard.c
#include <include/drivers/keyboard.h>
#include <include/drivers/k_language.h>
#include <include/printk.h>
#include <include/colors.h>
#include <include/string/string.h>
#include <include/ports.h>
#include <include/fs/fs.h>

#define KEYBOARD_PORT 0x60
#define STATUS_REGISTER 0x64
#define OUTPUT_BUFFER_FULL 0x01
#define SCREEN_BUFFER ((unsigned char *)0xb8000)
#define SCREEN_COLUMNS 80
#define SCREEN_ROWS 25
#define INPUT_BUFFER_SIZE 25
char input_buffer[INPUT_BUFFER_SIZE];


unsigned char read_scancode() {
    unsigned char scancode;
    __asm__ volatile ("inb %1, %0" : "=a"(scancode) : "Nd"(KEYBOARD_PORT));
    return scancode;
}




int caps_lock = 0;
int shift_pressed = 0;
volatile unsigned char last_scancode = 0;


int input_index = 0;

volatile int ctrl_pressed = 0;

extern int cursor_y;
extern int cursor_x;


void scroll_screen() {

    for (int i = 1; i < SCREEN_ROWS; i++) {
        for (int j = 0; j < SCREEN_COLUMNS * 2; j++) {
            SCREEN_BUFFER[(i - 1) * SCREEN_COLUMNS * 2 + j] =
                SCREEN_BUFFER[i * SCREEN_COLUMNS * 2 + j];
        }
    }

    for (int j = 0; j < SCREEN_COLUMNS * 2; j++) {
        SCREEN_BUFFER[(SCREEN_ROWS - 1) * SCREEN_COLUMNS * 2 + j] = 0;
    }

    cursor_y = SCREEN_ROWS - 1;
    cursor_x = 0;
}

void put_char(char c) {
    int pos = (cursor_y * SCREEN_COLUMNS + cursor_x) * 2;

    SCREEN_BUFFER[pos] = c;
    SCREEN_BUFFER[pos + 1] = 0x07;


    cursor_x++;
    if (cursor_x >= SCREEN_COLUMNS) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= SCREEN_ROWS) {
        scroll_screen();
    }
}

int process_input ()
{
    input_buffer[input_index] = '\0';

    if (strcmp(input_buffer, "main") == 0)
    {
        printk("THANKS GOD FOR ALL!", cursor_y++, GREEN);
    }

    else if (strcmp(input_buffer, "help") == 0)
    {
        cat("help.txt");
    }

    else if (strcmp(input_buffer, "") == 0)
    {
       cursor_y++;
    }


    else if (strcmp(input_buffer, "clear") == 0)
    {
        cursor_y = 0;
        clear_screen();
    }

    else if (strcmp(input_buffer, "pwd") == 0)
    {
        cursor_y = 0;
        pwd();
    }


    else if (strncmp(input_buffer, "mkdir ", 6) == 0)
    {
        const char *dirname = input_buffer + 6;
        cursor_y = cursor_y + 1;
        mkdir(dirname);
    }

    else if (strncmp(input_buffer, "cd ", 3) == 0)
    {
        const char *dirname = input_buffer + 3;
        cd(dirname);
    }


    else if (strncmp(input_buffer, "touch ", 6) == 0)
    {
        const char *dirname = input_buffer + 6;
        cursor_y = cursor_y + 1;
        printk("File created", cursor_y++, GREEN);
        touch(dirname, "");
    }

    else if (strncmp(input_buffer, "cat ", 4) == 0)
    {
        const char *dirname = input_buffer + 4;
        cursor_y = cursor_y + 1;
        cat(dirname);
    }




    else if (strcmp(input_buffer, "ls") == 0)
    {
        cursor_y = cursor_y + 1;
        list_items();
    }

    else
    {
        printk("ERR: Command not found", cursor_y++, RED);
    }

    input_index = 0;
    cursor_x = 0;
}

void keyboard_handler() {
  unsigned char scancode = read_scancode();
  unsigned char scancode1 = inb(0x60);
  if (scancode1 == 0x1D) { // Ctrl pressed
        ctrl_pressed = 1;
    } else if (scancode1 == 0x9D) { // Ctrl down
        ctrl_pressed = 0;
    } else if (scancode1 == 0x26 && ctrl_pressed) { // ctrl + l pressed
        cursor_y = 0;
        clear_screen();
    }

    if (scancode == last_scancode ) {
        return;
    }

    last_scancode = scancode;

    if (scancode & 0x80 ) {

        scancode &= 0x7F;


        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 0;
        }

        return;
    } else {

        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
            return;
        } else if (scancode == 0x3A) {
            caps_lock = !caps_lock;
            return;
        }
    }


    if (scancode < sizeof(scancode_to_ascii)) {
        char ascii = scancode_to_ascii[scancode];


        if ((caps_lock || shift_pressed) && ascii >= 'a' && ascii <= 'z') {
            ascii -= 32;
        } else if (shift_pressed && ascii >= '0' && ascii <= '9') {

            switch (ascii) {
                case '1': ascii = '!'; break;
                case '2': ascii = '@'; break;
                case '3': ascii = '#'; break;
                case '4': ascii = '$'; break;
                case '5': ascii = '%'; break;
                case '6': ascii = '^'; break;
                case '7': ascii = '&'; break;
                case '8': ascii = '*'; break;
                case '9': ascii = '('; break;
                case '0': ascii = ')'; break;
                case ',': ascii = ';'; break;
                case '.': ascii = ':'; break;
            }
        }


        if (ascii == '\b') {
            if (input_index > 0) {
                input_index--;
                if (cursor_x > 0) {
                    cursor_x--;
                } else if (cursor_y > 0) {
                    cursor_y--;
                    cursor_x = SCREEN_COLUMNS - 1;
                }

                int pos = (cursor_y * SCREEN_COLUMNS + cursor_x) * 2;
                SCREEN_BUFFER[pos] = ' ';
                SCREEN_BUFFER[pos + 1] = 0x07;
            }
        } else if (ascii == '\n') {
            process_input();
        } else if (ascii) {
            if (input_index < INPUT_BUFFER_SIZE - 1) {
                input_buffer[input_index++] = ascii;
                put_char(ascii);
            }
        }
    }
}
