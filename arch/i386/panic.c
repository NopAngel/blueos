#include <kernel/printk.h>
#include <kernel/colors.h>
#include <kernel/bg.h>
#include <kernel/vmcore_info.h>
#include <stdarg.h>

typedef unsigned int uint32_t;

static void halt_system() {
    __asm__ volatile("cli");
    for (;;) {
        __asm__ volatile("hlt");
    }
}


void k_panic(const char *file, int line, const char *reason, ...) {

    __asm__ volatile("cli");

    bg_clear(0x4F); 

    printk(0x4F, "\n  ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
    printk(0x4F, "  ::                !!! BLUEOS KERNEL PANIC !!!           ::\n");
    printk(0x4F, "  ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n");

    printk(0x4F, "  REASON:  ");
    
    va_list args;
    va_start(args, reason);

    vprintk(0x4F, reason, args); 
    va_end(args);

    printk(0x4F, "\n");

    if (file) {
        printk(0x4F, "  SOURCE:  %s\n", file);
        printk(0x4F, "  LINE:    %d\n", line);
    }

    printk(0x4F, "\n  CPU REGISTERS:\n");
    
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp_reg;
    __asm__ volatile("mov %%eax, %0" : "=r"(eax));
    __asm__ volatile("mov %%ebx, %0" : "=r"(ebx));
    __asm__ volatile("mov %%ecx, %0" : "=r"(ecx));
    __asm__ volatile("mov %%edx, %0" : "=r"(edx));
    __asm__ volatile("mov %%esi, %0" : "=r"(esi));
    __asm__ volatile("mov %%edi, %0" : "=r"(edi));
    __asm__ volatile("mov %%esp, %0" : "=r"(esp));
    __asm__ volatile("mov %%ebp, %0" : "=r"(ebp_reg));

    printk(0x4F, "  EAX: 0x%x  EBX: 0x%x  ECX: 0x%x  EDX: 0x%x\n", eax, ebx, ecx, edx);
    printk(0x4F, "  ESI: 0x%x  EDI: 0x%x  ESP: 0x%x  EBP: 0x%x\n", esi, edi, esp, ebp_reg);

    printk(0x4F, "\n  STACK BACKTRACE:\n");
    
    uint32_t *ebp = (uint32_t*)ebp_reg;
    for (int i = 0; i < 6 && ebp != 0; i++) {
        uint32_t eip = ebp[1]; 
        if (eip == 0) break;
        
        printk(0x4F, "    [%d] at 0x%x\n", i, eip);
     
        uint32_t *next_ebp = (uint32_t*)ebp[0];
        if (next_ebp <= ebp || (uint32_t)next_ebp > 0xC0000000) break; 
        ebp = next_ebp;
    }

    printk(0x4F, "\n  ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
    printk(0x4F, "  ::    SYSTEM HALTED. PLEASE RESTART YOUR COMPUTER.      ::\n");
    printk(0x4F, "  ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");

    halt_system();
}