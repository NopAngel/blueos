#include <blueos/printk.h>
#include <blueos/colors.h>
#include <blueos/bg.h>
#include <kernel/vmcore_info.h>
#include <stdarg.h>
#include <stdint.h> 
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


    printk(0x4F, "\n  CPU REGISTERS (x64):\n");
    
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp_reg;
    __asm__ volatile("mov %%rax, %0" : "=r"(rax));
    __asm__ volatile("mov %%rbx, %0" : "=r"(rbx));
    __asm__ volatile("mov %%rcx, %0" : "=r"(rcx));
    __asm__ volatile("mov %%rdx, %0" : "=r"(rdx));
    __asm__ volatile("mov %%rsi, %0" : "=r"(rsi));
    __asm__ volatile("mov %%rdi, %0" : "=r"(rdi));
    __asm__ volatile("mov %%rsp, %0" : "=r"(rsp));
    __asm__ volatile("mov %%rbp, %0" : "=r"(rbp_reg));

    printk(0x4F, "  RAX: 0x%x%x  RBX: 0x%x%x\n", (uint32_t)(rax >> 32), (uint32_t)rax, (uint32_t)(rbx >> 32), (uint32_t)rbx);
    printk(0x4F, "  RSP: 0x%x%x  RBP: 0x%x%x\n", (uint32_t)(rsp >> 32), (uint32_t)rsp, (uint32_t)(rbp_reg >> 32), (uint32_t)rbp_reg);


    printk(0x4F, "\n  STACK BACKTRACE:\n");
    
    uint64_t *rbp = (uint64_t*)rbp_reg;
    for (int i = 0; i < 6 && rbp != 0; i++) {
        uint64_t rip = rbp[1]; 
        if (rip == 0) break;
        
        printk(0x4F, "    [%d] at 0x%x%x\n", i, (uint32_t)(rip >> 32), (uint32_t)rip);
        
        uint64_t *next_rbp = (uint64_t*)rbp[0];
        
        if (next_rbp <= rbp) break; 
        rbp = next_rbp;
    }

    printk(0x4F, "\n  ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
    printk(0x4F, "  ::        SYSTEM HALTED. PLEASE RESTART YOUR COMPUTER.  ::\n");
    printk(0x4F, "  ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");

    halt_system();
}