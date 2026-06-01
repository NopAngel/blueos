#include <stdint.h>
#include <arch/riscv/sbi.h>
#include <kernel/colors.h>
#include <kernel/printk.h>
#include <kernel/hal.h>
struct sbiret {
    long error;
    long value;
};

void sbi_system_reset(uint32_t type, uint32_t reason) {
#if defined(RISCV) || defined(__riscv)
    register unsigned long a0 asm("a0") = (unsigned long)type;
    register unsigned long a1 asm("a1") = (unsigned long)reason;
    register unsigned long a6 asm("a6") = 0x00000000;
    register unsigned long a7 asm("a7") = 0x53525354;

    asm volatile ("ecall"
                  : "+r" (a0), "+r" (a1)
                  : "r" (a6), "r" (a7)
                  : "memory");
#elif defined(x86) || defined(__x86__)
    printk("Reset no implementado en x86 aún...\n");
    asm volatile("hlt");
#endif
}

void arch_cpu_halt() {
#if defined(x86) || defined(__x86__)
    asm volatile("hlt");
#elif defined(RISCV) || defined(__riscv)
    asm volatile("wfi");
#endif
}


int fat16_read_file(const char* path, char* buffer) {
    return -1;
}
