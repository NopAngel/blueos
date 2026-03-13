#include <drivers/keyboard.h>
#include <blueos/printk.h>
#include <blueos/colors.h>
#include <blueos/ports.h>
#include <lib/string.h>
#include <blueos/commands.h>

/* --- External References from commands.c --- */
extern shell_command_t commands_table[];
extern void list_matches(char* prefix);
extern void execute_shell_command(char* input);
extern void add_to_history(char* cmd);
extern void print_prompt();
extern char command_history[HISTORY_MAX][INPUT_BUFFER_SIZE];
extern int history_count;
extern int history_browse_index;

/* --- Global Keyboard State --- */
int tab_count = 0;
char input_buffer[INPUT_BUFFER_SIZE];
int input_index = 0;
int caps_lock = 0;
int shift_pressed = 0;
int ctrl_pressed = 0;
volatile unsigned char last_scancode = 0;
int keyboard_echo = 1;
int cursor_x = 0;
int cursor_y = 0;

char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 
    0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // f1-f12
};

/* --- Screen Helpers --- */

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

void clear_current_line() {
    while (input_index > 0) {
        handle_backspace();
    }
}

void load_history_cmd(int index) {
    clear_current_line();
    char* cmd = command_history[index % HISTORY_MAX];
    for (int i = 0; cmd[i] != '\0'; i++) {
        input_buffer[input_index++] = cmd[i];
        put_char(cmd[i], WHITE);
    }
}

/* --- Main Handler --- */

void keyboard_handler(struct registers *r) {
    unsigned char scancode = inb(KEYBOARD_PORT);
    pic_send_eoi(1); 

    if (scancode & 0x80) {
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

        case 0x48: // UP ARROW
            if (history_count > 0) {
                if (history_browse_index == -1) history_browse_index = (history_count - 1);
                else if (history_browse_index > 0 && history_browse_index > (history_count - HISTORY_MAX)) {
                    history_browse_index--;
                }
                load_history_cmd(history_browse_index);
            }
            return;

        case 0x50: // DOWN ARROW
            if (history_browse_index != -1) {
                if (history_browse_index < history_count - 1) {
                    history_browse_index++;
                    load_history_cmd(history_browse_index);
                } else {
                    history_browse_index = -1;
                    clear_current_line();
                }
            }
            return;

        case 0x0F: // TAB (Autocomplete)
            tab_count++;
            input_buffer[input_index] = '\0';
            int len = strlen(input_buffer);
            int matches = 0;
            const char* match_name = 0;

            for (int i = 0; commands_table[i].name != 0; i++) {
                if (strncmp(input_buffer, commands_table[i].name, len) == 0) {
                    match_name = commands_table[i].name;
                    matches++;
                }
            }

            if (matches == 1) {
                clear_current_line();
                input_index = 0;
                for (int i = 0; match_name[i] != '\0'; i++) {
                    input_buffer[input_index++] = match_name[i];
                    put_char(match_name[i], WHITE);
                }
                tab_count = 0;
            } else if (matches > 1 && tab_count >= 2) {
                list_matches(input_buffer);
                print_prompt();
                for(int i = 0; i < input_index; i++) put_char(input_buffer[i], WHITE);
                tab_count = 0;
            }
            return;
    }

    // Reset TAB count if any other key is pressed
    if (scancode != 0x0F) tab_count = 0;

    if (scancode < 128) {
        char ascii = scancode_to_ascii[scancode];
        if (!ascii) return;

        if ((caps_lock || shift_pressed) && ascii >= 'a' && ascii <= 'z') ascii -= 32;

        // Ctrl + L (Clear screen)
        if (ctrl_pressed && ascii == 'l') {
            clear_screen(); cursor_x = 0; cursor_y = 0;
            input_index = 0; update_cursor(0, 0);
            print_prompt();
            return;
        }

        if (shift_pressed) {
            if (ascii == '4') ascii = '$';
            else if (ascii == '1') ascii = '!'; 
            else if (ascii == '2') ascii = '@';
            else if (ascii == '3') ascii = '#';
            else if (ascii == '5') ascii = '%'; // Para el módulo en bc
            else if (ascii == '7') ascii = '&';
            else if (ascii == '8') ascii = '*'; // Multiplicación
            else if (ascii == '9') ascii = '(';
            else if (ascii == '0') ascii = ')';
            else if (ascii == '-') ascii = '_';
            else if (ascii == '=') ascii = '+'; // <--- EL MÁS IMPORTANTE PARA BC
            else if (ascii == '8') ascii = '*';
            else if (ascii == '/') ascii = '?'; 
            
            if (ascii >= 'a' && ascii <= 'z') ascii -= 32;

            
        } else if (caps_lock && ascii >= 'a' && ascii <= 'z') {
            ascii -= 32; 
        }

        if (ascii == '\b') {
            handle_backspace();
        } else if (ascii == '\n') {
            put_char('\n', WHITE);
            input_buffer[input_index] = '\0';
            
            if (input_index > 0) {
                add_to_history(input_buffer);
                execute_shell_command(input_buffer);
            } else {
                print_prompt();
            }
            
            input_index = 0;
            history_browse_index = -1;
        } else if (input_index < INPUT_BUFFER_SIZE - 1) {
            input_buffer[input_index++] = ascii;
            if (keyboard_echo) put_char(ascii, WHITE);
        }
    }
}