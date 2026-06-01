#include <kernel/printk.h>
#include <kernel/timer.h>
#include <kernel/arch.h>
#include <kernel/ports.h>
#include <kernel/hal.h>
#include <drivers/power.h>
#include <stdint.h>

extern uint32_t system_ticks;
extern unsigned int total_memory_kb;
extern char current_user[];
extern int tty_current();
extern char raw_get_char();


void k_panic(int code, const char* reason) {
    arch_disable_interrupts();

    uint32_t total_seconds = system_ticks / 100;
    uint32_t minutes = total_seconds / 60;
    uint32_t seconds = total_seconds % 60;
    uint32_t total_mb = total_memory_kb / 1024;

    printk("\nDUMP(int %d)\n", code);
    printk("{\n");
    printk("sys. log:\n");
    printk("Reasson: %s\n", reason ? reason : "Fatal exception in interrupt");
    printk("    + tty : tty%d\n\n", tty_current());


    printk("----> [ KERNEL PANIC ] <----\n");
    printk("The system will restart in 15 seconds or if you press any key\n\n");
    printk("}\n");

    for (int i = 0; i < 1500; i++) { 
       
        if (inb(0x64) & 1) {
            break; 
        }

        for(volatile int d=0; d<1000000; d++);
    }


    sys_reboot();
}
