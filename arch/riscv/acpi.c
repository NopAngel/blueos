#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stdint.h>

#define SYSCON_TEST_ADDR 0x100000

#define FINISHER_FAIL 0x3333
#define FINISHER_PASS 0x5555
#define FINISHER_RESET 0x7777

void acpi_init() {
  printk(
      "\033[32mSystem: Initializing RISC-V Power Control (MMIO 0x%x)\033[0m\n",
      SYSCON_TEST_ADDR);
  printk("\033[32mSystem: Ready to handle Shutdown/Reboot\033[0m\n");
}

void sys_shutdown() {
  printk("BlueOS: Shutting down...\n");

  volatile uint32_t *finisher = (uint32_t *)SYSCON_TEST_ADDR;

  *finisher = FINISHER_PASS;

  while (1) {
    __asm__ __volatile__("wfi");
  }
}

void sys_reboot() {
  printk("\nBlueOS: Rebooting...\n");

  volatile uint32_t *finisher = (uint32_t *)SYSCON_TEST_ADDR;

  *finisher = FINISHER_RESET;

  while (1) {
    __asm__ __volatile__("wfi");
  }
}