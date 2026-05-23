#include <kernel/ports.h>
#include <stdint.h>

void play_sound(uint32_t n_frequence) {
    if (n_frequence == 0) return;

    uint32_t div = 1193182 / n_frequence;

    disable_interrupts();

    outb(0x43, 0xB6);
    io_wait();

    outb(0x42, (uint8_t)(div & 0xFF));
    io_wait();
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));

    uint8_t tmp = inb(0x61);
    if ((tmp & 3) != 3) {
        outb(0x61, tmp | 3);
    }

    enable_interrupts();
}

void nosound() {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}
