#include <blueos/printk.h>
#include <blueos/colors.h>
#include <blueos/bg.h>
#include <kernel/vmcore_info.h>

typedef unsigned int uint32_t;


void k_panic(const char *reason, const char *file, int line) {
    __asm__ volatile("cli");


    //clear_screen(); 
    bg_clear(RED_WHITE);
    
    printk(RED_WHITE, "  ##################################################\n");
    printk(RED_WHITE, "  #                KERNEL PANIC!                   #\n");
    printk(RED_WHITE, "  ##################################################\n\n");

    printk(RED_WHITE, "  DETAIL: ");
    printk(RED_WHITE, "%s\n", reason);
    
    if (file) {
        printk(RED_WHITE, "  FILES: %s\n", file);
        printk(RED_WHITE, "  LINE:   %d\n\n", line);
    }

    #ifdef CONFIG_DEBUG
        dump_vmcoreinfo();
    #endif
   
    printk(CYAN, "  Stack Trace:\n");
    
    uint32_t *ebp;
    __asm__ volatile ("mov %%ebp, %0" : "=r" (ebp));

    for (int i = 0; i < 5 && ebp != 0; i++) {
        uint32_t eip = ebp[1]; 
        printk(RED_WHITE, "    [%d] 0x%x\n", i, eip);
        ebp = (uint32_t*)ebp[0]; 
    }

    printk(RED_WHITE, "\n  ##################################################\n");

    for (;;);
}