#include <config.h>
#include <drivers/keyboard.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <kernel/commands.h>
#include <kernel/ports.h>
#include <lib/string.h>

/* --- S390 Emulated State --- */
typedef struct {
    char line_buffer[INPUT_BUFFER_SIZE];
    int pos;
    int uppercase_only;
} s390_state_t;

static s390_state_t s390_kbd = { .pos = 0, .uppercase_only = 0 };


static char s390_decode(uint8_t code) {
    #if defined(I386) || defined(__x86_64__)
        static const char at_map[] = {
            0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
            '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
            0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
            '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
        };
        if (code < 58) return at_map[code];
    #elif defined(RISCV)
        return (char)code;
    #endif
    return 0;
}

char get_char() {
    // (Busy wait)
    while (head == tail) {
        asm volatile("hlt");
    }

    char c = keyboard_buffer[tail];
    tail = (tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

/**
 * keyboard_handler: universal driver
 */
void keyboard_handler() {
    uint8_t raw_code = 0;

    #if defined(I386) || defined(__x86_64__)
        if (!(inb(0x64) & 1)) return;
        raw_code = inb(0x60);
        if (raw_code & 0x80) return; // IGNORE: release
    #elif defined(RISCV)
        volatile uint8_t *uart_lsr = (uint8_t *)(0x10000000 + 5);
        volatile uint8_t *uart_dr  = (uint8_t *)(0x10000000 + 0);
        if (!(*uart_lsr & 0x01)) return; // No data ready
        raw_code = *uart_dr;
    #endif

    /* 2. Decodificación */
    char c = s390_decode(raw_code);
    if (c == 0) return;

    if (c == '\n' || c == '\r') {
        arch_put_char('\n', WHITE);
        s390_kbd.line_buffer[s390_kbd.pos] = '\0';

        if (s390_kbd.pos > 0) {
            execute_shell_command(s390_kbd.line_buffer);
        } else {
            print_prompt();
        }
        s390_kbd.pos = 0;
    }
    else if (c == '\b' || raw_code == 127) {
        if (s390_kbd.pos > 0) {
            s390_kbd.pos--;
            arch_put_char('\b', WHITE);
        }
    }
    else if (s390_kbd.pos < INPUT_BUFFER_SIZE - 1) {
        if (s390_kbd.uppercase_only && c >= 'a' && c <= 'z') {
            c -= 32;
        }

        s390_kbd.line_buffer[s390_kbd.pos++] = c;
        arch_put_char(c, WHITE);
    }
}
