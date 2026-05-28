
#include <kernel/printk.h>
#include <arch/riscv/plic.h>
#include <kernel/colors.h>

#define PLIC_BASE          0x0c000000
#define PLIC_PRIORITY      (PLIC_BASE + 0x0)
#define PLIC_PENDING       (PLIC_BASE + 0x1000)
#define PLIC_ENABLE        (PLIC_BASE + 0x2000)
#define PLIC_THRESHOLD     (PLIC_BASE + 0x200000)
#define PLIC_CLAIM         (PLIC_BASE + 0x200004)

void plic_write(uint32_t reg, uint32_t data) {
    volatile uint32_t *addr = (uint32_t *)(reg);
    *addr = data;
}

uint32_t plic_read(uint32_t reg) {
    volatile uint32_t *addr = (uint32_t *)(reg);
    return *addr;
}

void plic_complete(uint32_t irq) {
    plic_write(PLIC_CLAIM, irq);
}

uint32_t plic_claim(void) {
    return plic_read(PLIC_CLAIM);
}

void plic_init(void) {
    printk(GREEN, "PLIC: Initializing Platform-Level Interrupt Controller at %p...\n", PLIC_BASE);
    plic_write(PLIC_THRESHOLD, 0);

    printk(GREEN, "PLIC: Controller ready. Priority and Threshold set.\n");
}
