#include <blueos/ports.h>
#include <blueos/colors.h>
#include <blueos/printk.h>

void timer_init(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
    printk(GREEN, "info: PIT timer initialized at %d Hz\n", frequency);
}