#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "VT_KEYBOARD"

/* Key Modifiers State Flags Bitmask definition tracking */
static int g_modifier_shift = 0;
static int g_modifier_ctrl  = 0;
static int g_modifier_alt   = 0;

/* Simple US-ASCII Non-shifted Scancode Conversion Translation Table */
static const unsigned char kbd_us_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
     0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' '
};

/**
 * vt_kbd_process_scancode: Evaluates make/break codes passing arguments downstream into active VTs.
 */
void vt_kbd_process_scancode(uint8_t scancode) {
    /* Evaluate Key Release break code parameters (Bit 7 raised high) */
    if (scancode & 0x80) {
        uint8_t released_code = scancode & 0x7F;
        if (released_code == 0x2A || released_code == 0x36) g_modifier_shift = 0;
        if (released_code == 0x1D) g_modifier_ctrl = 0;
        if (released_code == 0x38) g_modifier_alt = 0;
        return;
    }

    /* Evaluate Make codes (Key pressed down) */
    switch (scancode) {
        case 0x2A: case 0x36: g_modifier_shift = 1; return;
        case 0x1D:           g_modifier_ctrl  = 1; return;
        case 0x38:           g_modifier_alt   = 1; return;
    }

    /* Convert structural hardware scancodes to standard ASCII strings array maps */
    if (scancode < 128) {
        unsigned char ascii_char = kbd_us_map[scancode];
        
        if (ascii_char != 0) {
            /* If shift modifier is flagged active, mutate characters to capital letters */
            if (g_modifier_shift && ascii_char >= 'a' && ascii_char <= 'z') {
                ascii_char -= 32;
            }
            
            printk("<7>[  %s ] Translated key: '%c' (Ctrl:%d, Alt:%d)\n", 
                   MODULE_NAME, ascii_char, g_modifier_ctrl, g_modifier_alt);
            
            /* Route directly to active VT streams -> vt_print_char(g_current_foreground_vt, ascii_char); */
        }
    }
}