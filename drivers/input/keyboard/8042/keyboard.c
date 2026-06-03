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
extern void print_prompt();

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

extern void kbd_process_char(char c);
extern void kbd_send_sequence(const char* s);

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
 * keyboard_handler - Punto de entrada de la IRQ1 (Teclado PS/2)
 */
void keyboard_handler() {
    uint8_t scancode = arch_get_scancode();
    if (scancode == 0) return;

    /* 1. Handle Function Keys (TTY Switch) */
    if (scancode >= KBD_F1 && scancode <= KBD_F7) {
        int target_tty = scancode - KBD_F1 + 1;
        tty_switch(target_tty);
        
        printk("\nCurrent TTY: %d.\n", target_tty);
        print_prompt();
        return;
    }

    /* 3. Handle Key Releases (Bit 7 set) */
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) shift_state = 0;
        return;
    }

    /* 4. Handle Modifiers (Shift) */
    if (scancode == 0x2A || scancode == 0x36) {
        shift_state = 1;
        return;
    }

    /* 5. Map Scancode to ASCII and process via Common Logic */
    char c = 0;
#if defined(x86)
    if (scancode < sizeof(kbd_map)) {
        c = kbd_map[scancode];
        if (shift_state && c >= 'a' && c <= 'z') c -= 32;
    }
#endif

    if (c != 0) {
        kbd_process_char(c);
    }
}
