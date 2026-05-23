#include <kernel/ports.h>

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

static int x = 0;
static int y = 0;
static unsigned char color = 0x0F;

static void set_cursor(int x, int y)
{
    unsigned short pos = y * MAX_COLS + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}

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

    y = MAX_ROWS - 1;
}

void vt100_init(void)
{
    unsigned short *video_memory = (unsigned short *) VIDEO_ADDRESS;
    int i;

    for (i = 0; i < MAX_ROWS * MAX_COLS; i++) {
        video_memory[i] = ' ' | (color << 8);
    }

    x = 0;
    y = 0;
    set_cursor(x, y);
}

void vt100_putc(char c)
{
    unsigned short *video_memory = (unsigned short *) VIDEO_ADDRESS;

    if (c == '\n') {
        x = 0;
        y++;
    } else if (c == '\r') {
        x = 0;
    } else if (c == '\b') {
        if (x > 0) {
            x--;
        }
    } else {
        video_memory[y * MAX_COLS + x] = c | (color << 8);
        x++;
    }

    if (x >= MAX_COLS) {
        x = 0;
        y++;
    }

    if (y >= MAX_ROWS) {
        scroll();
    }

    set_cursor(x, y);
}

void vt100_puts(const char *s)
{
    while (*s) {
        vt100_putc(*s++);
    }
}
