#include "../include/fs/fs.h"
#include "../include/printk.h"
#include "../include/colors.h"

#define MAX_TEXT_SIZE 1024
char nano_buffer[MAX_TEXT_SIZE];
char current_filename[30];
int nano_index = 0;
int is_nano_active = 0;

extern int cursor_x;
extern int cursor_y;
extern void clear_screen(); 
extern void put_char(char c);

void start_nano(const char* filename) {
    is_nano_active = 1;
    nano_index = 0;
    strcpy(current_filename, filename);
    memset(nano_buffer, 0, MAX_TEXT_SIZE);
    
    clear_screen();
    cursor_y = 0;
    printk(YELLOW, " TEXT EDITOR v1.0 | File: %s | Press ESC to Save & Exit\n", filename);
    printk(WHITE, "--------------------------------------------------------------------------------\n");
    cursor_y = 2;
    cursor_x = 0;
}

void nano_save_and_exit() {
    is_nano_active = 0;
    nano_buffer[nano_index] = '\0';
    
    touch(current_filename, nano_buffer);
    
    clear_screen();
    printk(GREEN, "File '%s' saved successfully via FS!\n", current_filename);
}

void nano_handle_key(char c) {
    if (c == 27) {
        nano_save_and_exit();
        return;
    }

    if (c == '\b') { 
        if (nano_index > 0) {
            nano_index--;
            nano_buffer[nano_index] = '\0';
        }
    } else if (nano_index < MAX_TEXT_SIZE - 1) {
        nano_buffer[nano_index++] = c;
        put_char(c);
    }
}