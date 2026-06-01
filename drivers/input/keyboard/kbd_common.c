#include <drivers/keyboard.h>
#include <drivers/tty.h>
#include <drivers/vt100.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <kernel/commands.h>

/* Shared terminal state */
// vt100_putc is used for output.

typedef struct {
    char buffer[INPUT_BUFFER_SIZE];
    int index;
    int echo;
    int enabled;
} kbd_common_state_t;

kbd_common_state_t kbd_state = { .index = 0, .echo = 1, .enabled = 1 };

/* Buffer circular para lectura bloqueante (get_char) */
#define KBD_QUEUE_SIZE 256
static char kbd_queue[KBD_QUEUE_SIZE];
static volatile int kbd_head = 0;
static volatile int kbd_tail = 0;

/**
 * get_char - Lee un caracter del buffer de forma bloqueante
 */
char get_char() {
    while (kbd_head == kbd_tail) {
        #if defined(x86)
            __asm__ volatile("hlt");
        #endif
    }
    char c = kbd_queue[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_QUEUE_SIZE;
    return c;
}

extern void execute_shell_command(char* input);
extern void print_prompt();

/**
 * handle_erase - Common backspace logic for terminal input
 */
static void handle_erase() {
    if (kbd_state.index > 0) {
        kbd_state.index--;
        vt100_putc('\b');
        vt100_putc(' ');
        vt100_putc('\b');
    }
}

/**
 * kbd_process_char - Processes ASCII characters (called by hardware drivers)
 */
void kbd_process_char(char c) {
    if (!kbd_state.enabled || c == 0) return;

    /* Add to circular buffer for blocking reads */
    int next = (kbd_head + 1) % KBD_QUEUE_SIZE;
    if (next != kbd_tail) {
        kbd_queue[kbd_head] = c;
        kbd_head = next;
    }

    switch (c) {
        case '\n':
        case '\r':
            vt100_putc('\n');
            kbd_state.buffer[kbd_state.index] = '\0';
            if (kbd_state.index > 0) {
                execute_shell_command(kbd_state.buffer);
            }
            print_prompt(); // Display prompt after processing command
            kbd_state.index = 0;
            break;

        case '\b':
        case 127:
            handle_erase();
            break;

        default:
            if (c >= 32 && c <= 126 && kbd_state.index < INPUT_BUFFER_SIZE - 1) {
                kbd_state.buffer[kbd_state.index++] = c;
                if (kbd_state.echo) {
                    vt100_putc(c);
                }
            }
            break;
    }
}