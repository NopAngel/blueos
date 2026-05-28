#include <kernel/printk.h>
#include <kernel/colors.h>
#include <kernel/syscall.h>


typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

// This is defined in kernel/printk.c, but we should probably move it to a proper header.
extern void clear_screen(void);

typedef struct {
    uint32_t gs, fs, es, ds;      
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; 
    uint32_t eip, cs, eflags, useresp, ss; 
} registers_t;

void syscall_handler(registers_t regs) {
    switch (regs.eax) {
        case SYS_PRINTK:
            // Handles printing a string to the console.
            // ecx: color
            // ebx: string pointer
            printk(regs.ecx, (const char*)regs.ebx);
            break;

        case SYS_CLEAR:
            // Clears the screen.
            clear_screen(); 
            break;

        case SYS_EXIT:
            // For now, just print an exit message.
            // In the future, this will terminate the current process.
            printk(YELLOW, "Process exited with code %d\n", regs.ebx);
            // Here we would halt or schedule the next process.
            break;

        default:
            printk(RED, "Unknown Syscall: %d\n", regs.eax);
            break;
    }
}
