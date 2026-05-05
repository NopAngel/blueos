#include <kernel/printk.h>
#include <kernel/colors.h>

struct registers {
    uint32_t ds, es, fs, gs;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

void isr_handler(struct registers regs) {
    printk(RED, "\n[ KERNEL PANIC ]\n");
    printk(WHITE, "Exception: %d | Error Code: %d\n", regs.int_no, regs.err_code);
    printk(GRAY, "EIP: 0x%x | CS: 0x%x | EFLAGS: 0x%x\n", regs.eip, regs.cs, regs.eflags);


    for(;;);
}
