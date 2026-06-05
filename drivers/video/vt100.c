#include <kernel/ports.h>
#include <stdint.h>

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

/* ANSI State Machine Definitions */
typedef enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_CSI
} ansi_state_t;

extern int cursor_x;
extern int cursor_y;
static unsigned char color = 0x0F;
static ansi_state_t current_state = STATE_NORMAL;
static char param_buffer[32];
static int param_ptr = 0;

/* ANSI Color Mapping (30-37) to VGA Colors */
static uint8_t ansi_to_vga[] = {
    0,      // 30: Black
    4,      // 31: Red
    2,      // 32: Green
    6,      // 33: Yellow/Brown
    1,      // 34: Blue
    5,      // 35: Magenta
    3,      // 36: Cyan
    7       // 37: White/Light Gray
};

/**
 * set_cursor: Updates the hardware cursor position via VGA ports.
 */
static void set_cursor(int x, int y)
{
    unsigned short pos = y * MAX_COLS + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}

/**
 * scroll: Moves all rows up by one and clears the last row.
 */
static void scroll(void)
{
    unsigned short *video_memory = (unsigned short *) VIDEO_ADDRESS;
    int i;

    for (i = 0; i < (MAX_ROWS - 1) * MAX_COLS; i++) {
        video_memory[i] = video_memory[i + MAX_COLS];
    }

    for (i = (MAX_ROWS - 1) * MAX_COLS; i < MAX_ROWS * MAX_COLS; i++) {
        video_memory[i] = ' ' | (color << 8);
    }

    // Reset the vertical cursor position to the last row after scrolling
    cursor_y = MAX_ROWS - 1;
}

/**
 * vt100_init: Clears the screen and resets cursor position.
 */
void vt100_init(void)
{
    unsigned short *video_memory = (unsigned short *) VIDEO_ADDRESS;
    int i;

    for (i = 0; i < MAX_ROWS * MAX_COLS; i++) {
        video_memory[i] = ' ' | (color << 8);
    }

    cursor_x = 0;
    cursor_y = 0;
    set_cursor(cursor_x, cursor_y);
}

/**
 * vt100_set_color: Updates the current global text color.
 */
void vt100_set_color(uint8_t new_color) {
    color = new_color;
}

/**
 * handle_ansi_csi: Parses and executes ANSI Control Sequence Introducer commands.
 */
static void handle_ansi_csi(char command)
{
    param_buffer[param_ptr] = '\0';

    // Parse numeric parameters separated by semicolons
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
        case 'm': // Select Graphic Rendition (Colors)
            for (int i = 0; i <= p_idx; i++) {
                int v = params[i];
                if (v == 0) color = 0x0F;       // Reset to default white
                else if (v == 1) color |= 0x08; // Bold/Bright bit
                else if (v >= 30 && v <= 37) color = (color & 0x08) | ansi_to_vga[v - 30];
                else if (v >= 90 && v <= 97) color = 0x08 | ansi_to_vga[v - 90];
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

        case 'J': // Erase Display (2 = entire screen)
            if (val == 2) vt100_init();
            break;

        case 'K': // Erase in Line (0 = from cursor to end)
            for (int i = cursor_x; i < MAX_COLS; i++)
                ((unsigned short *)VIDEO_ADDRESS)[cursor_y * MAX_COLS + i] = ' ' | (color << 8);
            break;
    }
}

void vt100_set_cursor_shape(uint8_t start_scan, uint8_t end_scan) {
    outb(0x3D4, 0x0A); // Cursor Start
    outb(0x3D5, start_scan);
    outb(0x3D4, 0x0B); // Cursor End
    outb(0x3D5, end_scan);
}


/**
 * vt100_putc: Processes a single character, handling ANSI sequences or raw VGA output.
 */
void vt100_putc(char c)
{
    switch (current_state) {
        case STATE_NORMAL:
            if (c == 27) { // ESC character
                current_state = STATE_ESC;
            } else if (c == '\n') {
                cursor_x = 0;
                cursor_y++;
            } else if (c == '\r') {
                cursor_x = 0;
            } else if (c == '\t') {
                cursor_x = (cursor_x + 8) & ~(7);
                if (cursor_x >= MAX_COLS) { cursor_x = 0; cursor_y++; }
            } else if (c == '\b') {
                if (cursor_x > 0) cursor_x--;
            } else {
                unsigned short *video_memory = (unsigned short *) VIDEO_ADDRESS;
                video_memory[cursor_y * MAX_COLS + cursor_x] = c | (color << 8);
                cursor_x++;
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
            if ((c >= '0' && c <= '9') || c == ';') {
                if (param_ptr < 31) param_buffer[param_ptr++] = c;
            } else {
                handle_ansi_csi(c);
                current_state = STATE_NORMAL;
            }
            break;
    }

    // Handle line overflow and scrolling
    if (cursor_x >= MAX_COLS) { cursor_x = 0; cursor_y++; }
    if (cursor_y >= MAX_ROWS) scroll();

    // Only update hardware cursor if we are in normal state to avoid flicker
    if (current_state == STATE_NORMAL) set_cursor(cursor_x, cursor_y);
}

/**
 * vt100_puts: Prints a null-terminated string using the vt100 engine.
 */
void vt100_puts(const char *s)
{
    while (*s) {
        vt100_putc(*s++);
    }
}