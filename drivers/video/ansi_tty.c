#include <drivers/ansi_tty.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <lib/string.h>

extern void arch_put_char(char c, unsigned int color);
extern int cursor_x;
extern int cursor_y;
extern void update_cursor();
extern void clear_screen();

#define MAX_COLS 80
#define MAX_ROWS 25

typedef enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_CSI
} ansi_state_t;

static ansi_state_t current_state = STATE_NORMAL;
static char param_buffer[32];
static int param_ptr = 0;
static uint8_t current_color = WHITE;

// Mapeo de colores ANSI (30-37) a colores VGA de BlueOS
static uint8_t ansi_to_vga[] = {
    0,              // 30
    RED,            // 31
    GREEN,          // 32
    YELLOW,         // 33
    BLUE,           // 34
    5,              // 35
    CYAN,           // 36
    GRAY            // 37
};

void ansi_tty_set_color(uint8_t color) {
    current_color = color;
}

void ansi_tty_init() {
    current_state = STATE_NORMAL;
    param_ptr = 0;
    current_color = WHITE;
}

static void handle_csi(char command) {
    param_buffer[param_ptr] = '\0';

    int params[4] = {0, 0, 0, 0};
    int p_idx = 0;
    char *ptr = param_buffer;

    while (*ptr && p_idx < 4) {
        if (*ptr >= '0' && *ptr <= '9') {
            params[p_idx] = params[p_idx] * 10 + (*ptr - '0');
        } else if (*ptr == ';') {
            p_idx++;
        }
        ptr++;
    }

    int val = params[0];

    switch (command) {
        case 'm':
            for (int i = 0; i <= p_idx; i++) {
                int v = params[i];
                if (v == 0) current_color = WHITE;
                else if (v == 1) current_color |= 0x08; // Bold/Bright
                else if (v >= 30 && v <= 37) current_color = (current_color & 0x08) | ansi_to_vga[v - 30];
                else if (v >= 90 && v <= 97) current_color = ansi_to_vga[v - 90] | 0x08;
            }
            break;
        case 'A': // Cursor Up
            cursor_y = (cursor_y > (val ? val : 1)) ? cursor_y - (val ? val : 1) : 0;
            break;
        case 'B': // Cursor Down
            cursor_y = (cursor_y + (val ? val : 1) < MAX_ROWS) ? cursor_y + (val ? val : 1) : MAX_ROWS - 1;
            break;
        case 'C': // Cursor Forward
            cursor_x = (cursor_x + (val ? val : 1) < MAX_COLS) ? cursor_x + (val ? val : 1) : MAX_COLS - 1;
            break;
        case 'D': // Cursor Backward
            cursor_x = (cursor_x > (val ? val : 1)) ? cursor_x - (val ? val : 1) : 0;
            break;
        case 'J': // Erase Display
            if (val == 2) clear_screen();
            break;
        case 'H': // Cursor Position (row;col)
        case 'f':
            if (p_idx >= 1) {
                cursor_y = (params[0] > 0) ? params[0] - 1 : 0;
                cursor_x = (params[1] > 0) ? params[1] - 1 : 0;
            } else {
                cursor_x = 0;
                cursor_y = 0;
            }
            break;
    }
    update_cursor();
}

void ansi_tty_putc(char c) {
    switch (current_state) {
        case STATE_NORMAL:
            if (c == 27) { // ESC
                current_state = STATE_ESC;
            } else {
                arch_put_char(c, current_color);
            }
            break;

        case STATE_ESC:
            if (c == '[') {
                current_state = STATE_CSI;
                param_ptr = 0;
            } else {
                current_state = STATE_NORMAL;
            }
            break;

        case STATE_CSI:
            if (c >= '0' && c <= '9' || c == ';') {
                if (param_ptr < 31) param_buffer[param_ptr++] = c;
            } else {
                handle_csi(c);
                current_state = STATE_NORMAL;
            }
            break;
    }
}

void ansi_tty_puts(const char* s) {
    while(*s) ansi_tty_putc(*s++);
}
