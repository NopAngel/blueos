#include <drivers/keyboard.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <kernel/colors.h>
#include <kernel/commands.h>
#include <lib/string.h>

/* --- Global State --- */
char input_buffer[INPUT_BUFFER_SIZE];
int input_index = 0;
int tab_count = 0;
int keyboard_echo = 1;

/* External references for Shell integration */
extern shell_command_t commands_table[];
extern char command_history[HISTORY_MAX][INPUT_BUFFER_SIZE];
extern int history_count;
extern int history_browse_index;

extern void list_matches(char* prefix);
extern void execute_shell_command(char* input);
extern void add_to_history(char* cmd);
extern void print_prompt(void);

/* --- x86 Specific Tables (Enabled via -DI386) --- */
#if defined(I386)
static char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};
static int shift_pressed = 0;
static int caps_lock = 0;
#endif

/**
 * handle_backspace: Removes last char from buffer and screen.
 */
void handle_backspace() {
    if (input_index > 0) {
        input_index--;
        /* Use arch_put_char to handle specific hardware backspace (ANSI vs VGA) */
        arch_put_char('\b', WHITE);
    }
}

/**
 * load_history_cmd: Clears line and prints a command from history.
 */
void load_history_cmd(int index) {
    while (input_index > 0) handle_backspace();
    char* cmd = command_history[index % HISTORY_MAX];
    for (int i = 0; cmd[i] != '\0'; i++) {
        input_buffer[input_index++] = cmd[i];
        arch_put_char(cmd[i], WHITE);
    }
}

/**
 * keyboard_handler: Central processing for all key events.
 */
void keyboard_handler() {
    char c = 0;

#if defined(RISCV)
    /* --- RISC-V UART Logic --- */
    uint8_t raw = arch_get_scancode();
    if (raw == 0) return;
    
    c = (char)raw;
    if (c == '\r') c = '\n';    /* Normalize Carriage Return to Newline */
    if (c == 127)  c = '\b';    /* Normalize Delete to Backspace */

#elif defined(I386)
    /* --- i386 Scancode Logic --- */
    uint8_t scancode = arch_get_scancode();
    
    /* Handle Key Releases (bit 7 set) */
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) shift_pressed = 0;
        return;
    }

    /* Handle Modifiers */
    if (scancode == 0x2A || scancode == 0x36) { shift_pressed = 1; return; }
    if (scancode == 0x3A) { caps_lock = !caps_lock; return; }
    
    /* Handle Arrows for History */
    if (scancode == 0x48 && history_count > 0) { /* UP */
        if (history_browse_index == -1) history_browse_index = history_count - 1;
        else if (history_browse_index > 0) history_browse_index--;
        load_history_cmd(history_browse_index);
        return;
    }

    /* Convert Scancode to ASCII */
    if (scancode < sizeof(scancode_to_ascii)) {
        c = scancode_to_ascii[scancode];
        if (shift_pressed && c >= 'a' && c <= 'z') c -= 32;
        if (caps_lock && c >= 'a' && c <= 'z') c -= 32;
    }
#endif

    if (c == 0) return;

    /* --- Common Shell Logic --- */
    
    /* 1. Tab Autocomplete */
    if (c == '\t') {
        tab_count++;
        input_buffer[input_index] = '\0';
        int matches = 0;
        const char* match_name = NULL;
        int len = strlen(input_buffer);

        for (int i = 0; commands_table[i].name != NULL; i++) {
            if (strncmp(input_buffer, commands_table[i].name, len) == 0) {
                match_name = commands_table[i].name;
                matches++;
            }
        }

        if (matches == 1) {
            while (input_index > 0) handle_backspace();
            input_index = 0;
            for (int i = 0; match_name[i] != '\0'; i++) {
                input_buffer[input_index++] = match_name[i];
                arch_put_char(match_name[i], WHITE);
            }
        } else if (matches > 1 && tab_count >= 2) {
            arch_put_char('\n', WHITE);
            list_matches(input_buffer);
            print_prompt();
            /* Restore current input on the new line */
            for(int i = 0; i < input_index; i++) arch_put_char(input_buffer[i], WHITE);
            tab_count = 0;
        }
        return;
    }

    tab_count = 0;

    /* 2. Newline / Enter Processing */
    if (c == '\n') {
        arch_put_char('\n', WHITE); /* Visual feedback */
        input_buffer[input_index] = '\0';
        
        if (input_index > 0) {
            add_to_history(input_buffer);
            execute_shell_command(input_buffer);
        } else {
            print_prompt(); /* Just a new line if empty */
        }
        input_index = 0;
        history_browse_index = -1;
    } 
    /* 3. Backspace */
    else if (c == '\b') {
        handle_backspace();
    } 
    /* 4. Printable Characters */
    else if (c >= 32 && c <= 126 && input_index < INPUT_BUFFER_SIZE - 1) {
        input_buffer[input_index++] = c;
        if (keyboard_echo) {
            arch_put_char(c, WHITE); /* THIS FIXES THE INVISIBLE TYPING */
        }
    }
}