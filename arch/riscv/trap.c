#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stdint.h>

extern void trap_entry(void);

void trap_init(void) {
  __asm__ volatile("csrw stvec, %0" : : "r"(trap_entry));
  printk("\033[32m[  OK  ] TRAP: Vector Base Address Register (stvec) "
         "loaded.\033[0m\n");
}