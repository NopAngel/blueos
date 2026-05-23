#include <stdint.h>

void delay_us(uint32_t us) {
#if defined(__riscv)
    uint64_t start, current;
    uint64_t ticks = us * 10; 

    asm volatile("rdtime %0" : "=r"(start));
    do {
        asm volatile("rdtime %0" : "=r"(current));
    } while ((current - start) < ticks);

#elif defined(__x86__)
    for (uint32_t i = 0; i < us * 100; i++) {
        asm volatile("nop");
    }
#endif
}