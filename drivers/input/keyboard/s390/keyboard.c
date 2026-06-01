#include <config.h>
#include <drivers/keyboard.h>
#include <kernel/printk.h>
#include <kernel/ports.h>
#include <kernel/colors.h>
#include <kernel/commands.h>
#include <lib/string.h>
#include <drivers/tty.h>

extern void kbd_process_char(char c);
extern void print_prompt();

/**
 * arch_read_raw - Platform-specific hardware abstraction
 */
static uint8_t arch_read_raw(void) {
#if defined(x86) || defined(__x86_64__)
    /* x86: Check if data is ready in PS/2 Controller (Bit 0 of status reg 0x64) */
    if (!(inb(0x64) & 0x01)) return 0;
    uint8_t scancode = inb(0x60);
    /* Ignore key release events for now (Bit 7) */
    if (scancode & 0x80) return 0;
    return scancode;
#elif defined(__riscv)
    /* RISC-V: Use the UART driver function we fixed earlier */
    extern int arch_get_char();
    int c = arch_get_char();
    return (c == -1) ? 0 : (uint8_t)c;
#else
    return 0;
#endif
}

/**
 * s390_decode - Maps raw hardware codes to ASCII
 */
static char decode_code(uint8_t code) {
    if (code == 0) return 0;

#if defined(x86) || defined(__x86_64__)
    static const char at_map[] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    if (code < sizeof(at_map)) return at_map[code];
#elif defined(__riscv)
    /* RISC-V UART typically sends ASCII directly */
    char c = (char)code;
    if (c == '\r') return '\n'; // Normalize Enter
    return c;
#endif
    return 0;
}

/**
 * keyboard_handler - Main IRQ/Polling entry point
 */
void keyboard_handler() {
    uint8_t raw = arch_read_raw();
    if (raw == 0) return;

#if defined(x86) || defined(__x86_64__)
    if (raw >= KBD_F1 && raw <= KBD_F7) {
        int target_tty = raw - KBD_F1 + 1;
        tty_switch(target_tty);
        printk("\nCurrent TTY: %d.\n", target_tty);
        return;
    }
#endif



    /* 2. Decode scancode to ASCII */
    char c = decode_code(raw);
    if (c == 0) return;

    /* 4. Process character via common logic */
    kbd_process_char(c);
}
