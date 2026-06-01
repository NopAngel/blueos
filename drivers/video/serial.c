#include <drivers/serial.h>
#include <kernel/ports.h>

void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x01); // Low byte
    outb(COM1 + 1, 0x00); // High byte
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

static int is_transmit_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_putc(char c) {
    if (c == '\n') {
        while (is_transmit_empty() == 0);
        outb(COM1, '\r');
    }

    while (is_transmit_empty() == 0);
    outb(COM1, c);
}

void serial_puts(const char* str) {
    while (*str) {
        serial_putc(*str++);
    }
}
