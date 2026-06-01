#include <kernel/io.h>
#include <kernel/printk.h>

#define PS2_DATA     0x60
#define PS2_STATUS   0x64
#define PS2_COMMAND  0x64

uint8_t mouse_cycle = 0;
int8_t mouse_byte[3];
int mouse_x = 0, mouse_y = 0;

void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) if ((inb(PS2_STATUS) & 1) == 1) return;
    } else {
        while (timeout--) if ((inb(PS2_STATUS) & 2) == 0) return;
    }
}

void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(PS2_COMMAND, 0xD4);
    mouse_wait(1);
    outb(PS2_DATA, data);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return inb(PS2_DATA);
}

void mouse_init() {
    uint8_t status;

    mouse_wait(1);
    outb(PS2_COMMAND, 0xA8);

    mouse_wait(1);
    outb(PS2_COMMAND, 0x20);
    mouse_wait(0);
    status = (inb(PS2_DATA) | 2);
    mouse_wait(1);
    outb(PS2_COMMAND, 0x60);
    mouse_wait(1);
    outb(PS2_DATA, status);

    mouse_write(0xF6);
    mouse_read();
    mouse_write(0xF4);
    mouse_read();

    printk("[Mouse] Initialized PS/2 Mouse\n");
}

void mouse_handler(struct trap_frame *tf) {
    uint8_t status = inb(PS2_STATUS);

    if (!(status & 0x20)) return;

    mouse_byte[mouse_cycle++] = inb(PS2_DATA);

    if (mouse_cycle == 3) {
        mouse_cycle = 0;


        int rel_x = mouse_byte[1];
        int rel_y = mouse_byte[2];

        if (mouse_byte[0] & 0x10) rel_x |= 0xFFFFFF00;
        if (mouse_byte[0] & 0x20) rel_y |= 0xFFFFFF00;

        mouse_x += rel_x;
        mouse_y -= rel_y;

        printk("\rMouse: X=%d Y=%d      ", mouse_x, mouse_y);
    }
}
