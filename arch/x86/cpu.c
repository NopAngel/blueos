#include <stdint.h>
#include <kernel/printk.h>

#define CR4_PAE (1 << 5)
#define MSR_EFER 0xC0000080
#define EFER_NXE (1 << 11) // NX Enable

void cpu_enable_pae_nx(void) {
    uint32_t cr4;
    
    // 1. Leer CR4, activar el bit 5 (PAE) y escribirlo de vuelta
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= CR4_PAE;
    asm volatile("mov %0, %%cr4" :: "r"(cr4));
    
    // 2. Activar el bit NXE en el registro de características extendidas (EFER)
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(MSR_EFER));
    low |= EFER_NXE;
    asm volatile("wrmsr" :: "a"(low), "d"(high), "c"(MSR_EFER));
    
    printk("[CPU]: PAE and NX (No-Execute) hardware protection enabled.\n");
}