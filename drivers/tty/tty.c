/*
 * BlueOS - Universal TTY Driver (Multi-Console Support)
 * * This driver manages multiple virtual consoles (tty1-tty7) and implements
 * the POSIX Line Discipline (ICANON, ECHO, ISIG).
 */

#include <drivers/tty.h>
#include <drivers/keyboard.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <mm/memory.h>

extern int current_user_index;
extern char current_user[];

extern int cursor_x;
extern int cursor_y;
extern void update_cursor();

#define MAX_TTYS 7
#define TTY_BUFFER_SIZE 2048
#define CTRL(c) ((c) & 0x1F)



/* Global array of virtual terminals */
static tty_t v_ttys[MAX_TTYS];
static int active_tty_index = 0; /* Index in the array (0 to MAX_TTYS-1) */

/**
 * tty_init: Initializes all virtual consoles.
 * @param hw_write: The hardware output function (e.g., VGA or UART).
 */
void tty_init(void (*hw_write)(char)) {
    for (int i = 0; i < MAX_TTYS; i++) {
        memset(&v_ttys[i], 0, sizeof(tty_t));

        v_ttys[i].id = i + 1;
        v_ttys[i].hw_write = hw_write;

        /* Default Linux-like configuration */
        v_ttys[i].conf.c_lflag = ICANON | ECHO | ISIG;

        /* Setup default control characters */
        v_ttys[i].conf.c_cc[VINTR]  = CTRL('c');
        v_ttys[i].conf.c_cc[VERASE] = '\b';
        v_ttys[i].conf.c_cc[VKILL]  = CTRL('u');
        v_ttys[i].conf.c_cc[VEOF]   = CTRL('d');
    }
    vfs_touch("/dev/tty1", 0); // Create device nodes for TTYs
    vfs_touch("/dev/tty2", 0);
    vfs_touch("/dev/tty3", 0);
    vfs_touch("/dev/tty4", 0);
    vfs_touch("/dev/tty5", 0);
    vfs_touch("/dev/tty6", 0);
    vfs_touch("/dev/tty7", 0);

    boot_msg("TTY", "inited....\n", 0);
}

/**
 * tty_current: Returns the ID of the TTY currently in use.
 * @return integer (1 for /dev/tty1, etc.)
 */
int tty_current(void) {
    return v_ttys[active_tty_index].id;
}

/**
 * tty_get_all_names: Copies a list of all TTY names into the provided buffer.
 */
void tty_get_all_names(char *out_buf) {
    out_buf[0] = '\0';
    char tmp[10];

    for (int i = 0; i < MAX_TTYS; i++) {
        strcat(out_buf, "tty");
        /* Simple integer to string conversion for IDs 1-7 */
        tmp[0] = (char)(v_ttys[i].id + '0');
        tmp[1] = (i == MAX_TTYS - 1) ? '\0' : ' ';
        tmp[2] = '\0';
        strcat(out_buf, tmp);
    }
}

/**
 * tty_input: Entry point for raw characters from Keyboard/Serial.
 * Processes characters based on the active TTY's configuration.
 */
void tty_input(char c) {
    tty_t *tty = &v_ttys[active_tty_index];

    /* 1. Signal Handling (ISIG) */
    if (tty->conf.c_lflag & ISIG) {
        if (c == tty->conf.c_cc[VINTR]) {
            printk("\n^C - Interrupt Signal (SIGINT)\n");
            return;
        }
    }

    /* 2. Canonical Mode (Line Buffering) */
    if (tty->conf.c_lflag & ICANON) {
        /* Backspace logic */
        if (c == tty->conf.c_cc[VERASE] || c == 127) {
            if (tty->count > 0) {
                tty->head = (tty->head - 1) % TTY_BUFFER_SIZE;
                tty->count--;
                if (tty->conf.c_lflag & ECHO) {
                    tty->hw_write('\b');
                    tty->hw_write(' ');
                    tty->hw_write('\b');
                }
            }
            return;
        }

        /* Kill Line (Ctrl+U) */
        if (c == tty->conf.c_cc[VKILL]) {
            while (tty->count > 0 && tty->buffer[(tty->head - 1) % TTY_BUFFER_SIZE] != '\n') {
                tty_input(tty->conf.c_cc[VERASE]);
            }
            return;
        }
    }

    /* 3. Insert into circular buffer */
    if (tty->count < TTY_BUFFER_SIZE) {
        if (c == '\r') c = '\n';

        tty->buffer[tty->head] = c;
        tty->head = (tty->head + 1) % TTY_BUFFER_SIZE;
        tty->count++;

        /* 4. Echo to hardware */
        if (tty->conf.c_lflag & ECHO) {
            tty->hw_write(c);
        }
    }
}

/**
 * tty_read: Reads data from the active TTY buffer.
 * In Canonical mode, it waits until a newline is found.
 */
int tty_read(char* user_buf, int n) {
    tty_t *tty = &v_ttys[active_tty_index];
    uint32_t i = 0;

    while (i < n && tty->count > 0) {
        char c = tty->buffer[tty->tail];

        user_buf[i++] = c;
        tty->tail = (tty->tail + 1) % TTY_BUFFER_SIZE;
        tty->count--;

        /* Stop reading at newline in Canonical mode */
        if ((tty->conf.c_lflag & ICANON) && c == '\n') {
            break;
        }
    }
    return i;
}

/**
 * tty_write: Sends a string to the active TTY output.
 */
void tty_write(const char* data, int n) {
    tty_t *tty = &v_ttys[active_tty_index];
    for (int i = 0; i < n; i++) {
        tty->hw_write(data[i]);
    }
}

/**
 * tty_switch: Changes the foreground TTY.
 * @param id: TTY number to switch to (1 to MAX_TTYS).
 */
void tty_switch(int id) {
    if (id < 1 || id > MAX_TTYS || id == tty_current()) return;

    uint16_t *video_mem = (uint16_t *)0xB8000;
    tty_t *old_tty = &v_ttys[active_tty_index];
    tty_t *new_tty = &v_ttys[id - 1];

    for (int i = 0; i < (VIDEO_SIZE / 2); i++) {
        old_tty->screen_buffer[i] = video_mem[i];
    }
    old_tty->saved_cursor_x = cursor_x;
    old_tty->saved_cursor_y = cursor_y;

    active_tty_index = id - 1;

    for (int i = 0; i < (VIDEO_SIZE / 2); i++) {
        video_mem[i] = new_tty->screen_buffer[i];
    }
    cursor_x = new_tty->saved_cursor_x;
    cursor_y = new_tty->saved_cursor_y;
    current_user_index = -1;
    mm_memset(current_user, 0, 32);
    update_cursor();
}
