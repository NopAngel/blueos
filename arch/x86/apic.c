#include <arch/x86/apic.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

void apic_write(uint32_t reg, uint32_t data) {
    volatile uint32_t *addr = (uint32_t *)(APIC_BASE_ADDR + reg);
    *addr = data;
}

uint32_t apic_read(uint32_t reg) {
    volatile uint32_t *addr = (uint32_t *)(APIC_BASE_ADDR + reg);
    return *addr;
}


void apic_eoi(void) {
    apic_write(APIC_EOI, 0);
}

void apic_init(void) {
    printk("\033[32mAPIC: Initializing Local APIC at %p...\033[0m\n", APIC_BASE_ADDR);

    apic_write(APIC_SVR, apic_read(APIC_SVR) | 0x1FF);

    apic_write(APIC_TPR, 0);

    printk("\033[32mAPIC: Local APIC ID: %x\033[0m\n", apic_read(APIC_ID) >> 24);
}