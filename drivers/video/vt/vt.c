#include <kernel/printk.h>
#include <kernel/colors.h>
#include <kernel/errno.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "VT_CORE"
#define VT_MAX_LINES 25
#define VT_MAX_COLS  80

typedef struct {
    int      vt_number;
    uint32_t current_row;
    uint32_t current_col;
    uint8_t  text_attribute; /* Standard VGA foreground/background byte color pack */
    int      visual_mode;    /* 0 = Text-Mode, 1 = Graphics Framebuffer mapping */
} virtual_terminal_t;

static virtual_terminal_t g_vt_channels[4]; /* 4 Virtual Terminals mapping workspace */
static int g_current_foreground_vt = 0;

/**
 * vt_print_char: Parses character tokens and advances screen metrics.
 */
void vt_print_char(int vt_id, char c) {
    if (vt_id >= 4) return;
    virtual_terminal_t *vt = &g_vt_channels[vt_id];

    if (c == '\n') {
        vt->current_row++;
        vt->current_col = 0;
    } else if (c == '\r') {
        vt->current_col = 0;
    } else {
        /* If this is the active console in the screen foreground, push to hardware video matrix */
        if (vt_id == g_current_foreground_vt) {
            // uint16_t *vga_mem = (uint16_t*)0xB8000;
            // vga_mem[vt->current_row * VT_MAX_COLS + vt->current_col] = (vt->text_attribute << 8) | c;
        }
        vt->current_col++;
    }

    /* Wrap around scroll line constraints validation handles */
    if (vt->current_row >= VT_MAX_LINES) {
        vt->current_row = VT_MAX_LINES - 1;
        /* Trigger hardware vertical memory scroll engine shift row down */
    }
}

/**
 * vt_allocate: Pre-configures structures inside active kernel layouts.
 */
void vt_allocate(void) {
    for (int i = 0; i < 4; i++) {
        g_vt_channels[i].vt_number = i;
        g_vt_channels[i].current_row = 0;
        g_vt_channels[i].current_col = 0;
        g_vt_channels[i].text_attribute = 0x07; /* Classic light-grey on black canvas background */
        g_vt_channels[i].visual_mode = 0;
    }
    g_current_foreground_vt = 0;
    boot_msg(MODULE_NAME, "Virtual Terminal management layer initialization complete.", 0);
}