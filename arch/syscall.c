#include <include/printk.h>
#include <include/colors.h>


typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

#define SYS_PRINTK 1
#define SYS_CLEAR  2

typedef struct {
    uint32_t gs, fs, es, ds;      
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; 
    uint32_t eip, cs, eflags, useresp, ss; 
} registers_t;

void syscall_handler(registers_t regs) {
    switch (regs.eax) {
        case SYS_PRINTK:
            printk(regs.ecx, (const char*)regs.ebx);
            break;

        case SYS_CLEAR:
            clear_screen();
            break;

        default:
            printk(RED, "Unknown Syscall: %d\n", regs.eax);
            break;
    }
}