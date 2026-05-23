#include <config.h>
#include <drivers/keyboard.h>
#include <drivers/tty.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <kernel/commands.h>

#if defined(x86)
#include <lib/string.h>
#include <kernel/ports.h>
#endif

extern void _blueos_banner();

/* --- Scan Codes for Function Keys (x86) --- */
#define KBD_F1 0x3B
#define KBD_F2 0x3C
#define KBD_F3 0x3D
#define KBD_F4 0x3E
#define KBD_F5 0x3F
#define KBD_F6 0x40
#define KBD_F7 0x41

#define KEYBOARD_BUFFER_SIZE 256
#define INPUT_BUFFER_SIZE    256

extern int current_user_index;
extern char current_user[];

/* --- TTY-Style State for Terminal Discipline --- */
typedef struct {
    char buffer[INPUT_BUFFER_SIZE];
    int index;
    int echo;           /* If 0, don't print while typing (e.g. passwords) */
    int raw_mode;       /* If 1, send every char immediately without waiting for Enter */
    int enabled;
} tty_state_t;

static tty_state_t kbd_tty = { .index = 0, .echo = 1, .raw_mode = 0, .enabled = 1 };

/* Global buffer for characters ready to be read */
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static int head = 0;
static int tail = 0;

extern void execute_shell_command(char* input);
extern void print_prompt();

/**
 * get_char - Blocking read
 */
char get_char() {
    while (head == tail) {
#if defined(x86)
        asm volatile("hlt");
#elif defined(__riscv)
        asm volatile("wfi");
#endif
    }
    char c = keyboard_buffer[tail];
    tail = (tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

#if defined(x86)
static const char kbd_map[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};
static int shift_state = 0;
#endif

/**
 * handle_erase - Manages visual and logical backspace
 */
static void handle_erase() {
    if (kbd_tty.index > 0) {
        kbd_tty.index--;
        arch_put_char('\b', WHITE);
        arch_put_char(' ', WHITE);
        arch_put_char('\b', WHITE);
    }
}

/**
 * process_character - The core "Terminal Discipline" logic
 */
static void process_character(char c) {
    if (c == 0) return;

    switch (c) {
        case '\n':
        case '\r':
            arch_put_char('\n', WHITE);
            kbd_tty.buffer[kbd_tty.index] = '\0';
            if (kbd_tty.index > 0) {
                execute_shell_command(kbd_tty.buffer);
            } else {
                print_prompt();
            }
            kbd_tty.index = 0;
            break;

        case '\b':
        case 127:
            handle_erase();
            break;

        default:
            if (c >= 32 && c <= 126 && kbd_tty.index < INPUT_BUFFER_SIZE - 1) {
                kbd_tty.buffer[kbd_tty.index++] = c;
                if (kbd_tty.echo) {
                    arch_put_char(c, WHITE);
                }
            }
            break;
    }
}

/**
 * keyboard_handler - Main entry point called by IRQ
 */
void keyboard_handler() {
    if (!kbd_tty.enabled) return;

    uint8_t scancode = arch_get_scancode();
    if (scancode == 0) return;

    char c = 0;

#if defined(x86) || defined(__x86_64__)
    /* 1. Handle TTY Switching (F1-F7) */
    if (scancode >= KBD_F1 && scancode <= KBD_F7) {
        int target_tty = scancode - KBD_F1 + 1;
        tty_switch(target_tty);
        
        current_user[31] = '\0'; // Safety null terminate
        kbd_tty.index = 0;      // Reset line buffer
        
        printk(CYAN, "\nCurrent TTY: %d. User: %s\n\n", target_tty, current_user);
        print_prompt();
        return; 
    }

    /* 2. Handle Key Releases (Bit 7 set) */
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) shift_state = 0;
        return;
    }

    /* 3. Handle Modifiers (Shift) */
    if (scancode == 0x2A || scancode == 0x36) { 
        shift_state = 1; 
        return; 
    }

    /* 4. Map Scancode to ASCII */
    if (scancode < sizeof(kbd_map)) {
        c = kbd_map[scancode];
        if (shift_state && c >= 'a' && c <= 'z') c -= 32;
    }

#elif defined(__riscv)
    /* RISC-V UART normalization */
    c = (char)scancode;
    if (c == '\r') c = '\n';
    if (c == 127)  c = '\b';
#endif

    if (c != 0) {
        process_character(c);
    }
}