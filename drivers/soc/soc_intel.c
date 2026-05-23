#include <stdint.h>
#include <kernel/io.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <drivers/soc_intel.h>


soc_info_t sys_soc;

void soc_init() {
    uint32_t eax, ebx, ecx, edx;

    asm volatile("cpuid" 
                 : "=b"(ebx), "=c"(ecx), "=d"(edx) 
                 : "a"(0));

    uint32_t* vendor_ptr = (uint32_t*)sys_soc.vendor;
    vendor_ptr[0] = ebx;
    vendor_ptr[1] = edx;
    vendor_ptr[2] = ecx;
    sys_soc.vendor[12] = '\0';

    asm volatile("cpuid" 
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                 : "a"(1));

    sys_soc.family = (eax >> 8) & 0xF;
    sys_soc.model  = (eax >> 4) & 0xF;

    printk(CYAN, "[SoC] %s detected\n", sys_soc.vendor);
    printk(WHITE, "      Family: %d, Model: %d\n", sys_soc.family, sys_soc.model);
}

void soc_reset() {
    printk(RED, "[SoC] Resetting system...\n");
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
}