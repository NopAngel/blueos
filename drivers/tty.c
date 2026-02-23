#include <drivers/tty.h>
#include <blueos/printk.h>
#include <blueos/colors.h>

struct tty_struct tty0;

void console_write(const char *buf, size_t n) {
    for (size_t i = 0; i < n; i++) {
        putchar(buf[i], WHITE); 
    }
}

void tty_init() {
    tty0.index = 0;
    tty0.head = tty0.tail = tty0.count = 0;
    tty0.echo = 1;
    tty0.raw_mode = 0;
    tty0.write = console_write;
    
    printk(GREEN, "[  OK  ] TTY driver initialized: tty0\n");
}

void tty_receive_char(struct tty_struct *tty, char c) {
    if (tty->count < TTY_BUFFER_SIZE) {
        tty->input_buffer[tty->head] = c;
        tty->head = (tty->head + 1) % TTY_BUFFER_SIZE;
        tty->count++;

        if (tty->echo) {
            tty->write(&c, 1); 
        }
    }
}

int tty_read(struct tty_struct *tty, char *buf, size_t n) {
    size_t i = 0;
    while (i < n && tty->count > 0) {
        buf[i] = tty->input_buffer[tty->tail];
        tty->tail = (tty->tail + 1) % TTY_BUFFER_SIZE;
        tty->count--;
        i++;
    }
    return i;
}