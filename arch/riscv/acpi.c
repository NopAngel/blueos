#include <kernel/printk.h>
#include <kernel/colors.h>
#include <stdint.h>

#define SYSCON_TEST_ADDR 0x100000

#define FINISHER_FAIL    0x3333
#define FINISHER_PASS    0x5555
#define FINISHER_RESET   0x7777

void acpi_init() {
    printk(GREEN, "System: Initializing RISC-V Power Control (MMIO 0x%x)\n", SYSCON_TEST_ADDR);
    printk(GREEN, "System: Ready to handle Shutdown/Reboot\n");
}

void sys_shutdown() {
    printk(RED, "BlueOS: Shutting down...\n");

    volatile uint32_t *finisher = (uint32_t *)SYSCON_TEST_ADDR;

    *finisher = FINISHER_PASS;

    while(1) {
        __asm__ __volatile__("wfi");
    }
}

void sys_reboot() {
    printk(YELLOW, "\nBlueOS: Rebooting...\n");

    volatile uint32_t *finisher = (uint32_t *)SYSCON_TEST_ADDR;

    *finisher = FINISHER_RESET;

    while(1) {
        __asm__ __volatile__("wfi");
    }
}