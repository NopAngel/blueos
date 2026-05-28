#include <stdint.h>

uint32_t system_ticks = 0;

#define CLINT_BASE 0x2000000
#define MTIMECMP   (CLINT_BASE + 0x4000)
#define MTIME      (CLINT_BASE + 0xBFF8)

void timer_set_next(uint64_t delta) {
    uint64_t now = *(volatile uint64_t*)MTIME;
    *(volatile uint64_t*)MTIMECMP = now + delta;
}

void timer_init(void) {
    timer_set_next(100000);

    asm volatile("csrs mie, %0" :: "r"(1 << 7));
}
