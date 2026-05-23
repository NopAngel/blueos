#include <kernel/printk.h>
#include <kernel/colors.h>
#include <stdint.h>

extern void trap_entry(void); 




void trap_init(void) {
    __asm__ volatile("csrw stvec, %0" : : "r"(trap_entry));
    printk(GREEN, "[  OK  ] TRAP: Vector Base Address Register (stvec) loaded.\n");
}