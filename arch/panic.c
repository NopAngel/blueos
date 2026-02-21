#include <include/printk.h>
#include <include/colors.h>

typedef unsigned int uint32_t;


void k_panic(const char *reason, const char *file, int line) {
    __asm__ volatile("cli");


    clear_screen(); 
    
    printk(RED, "  ##################################################\n");
    printk(RED, "  #                KERNEL PANIC!                   #\n");
    printk(RED, "  ##################################################\n\n");

    printk(WHITE, "  DETALLE: ");
    printk(YELLOW, "%s\n", reason);
    
    if (file) {
        printk(WHITE, "  ARCHIVO: %s\n", file);
        printk(WHITE, "  LINEA:   %d\n\n", line);
    }

   
    printk(CYAN, "  Stack Trace (Rastro de llamadas):\n");
    
    uint32_t *ebp;
    __asm__ volatile ("mov %%ebp, %0" : "=r" (ebp));

    for (int i = 0; i < 5 && ebp != 0; i++) {
        uint32_t eip = ebp[1]; 
        printk(WHITE, "    [%d] 0x%x\n", i, eip);
        ebp = (uint32_t*)ebp[0]; 
    }

    printk(RED, "\n  ##################################################\n");
    printk(WHITE, "  SISTEMA DETENIDO. REINICIA MANUALMENTE.");

    for (;;);
}