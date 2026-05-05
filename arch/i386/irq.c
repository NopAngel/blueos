#include <kernel/printk.h>
#include <kernel/panic.h>
#include <kernel/colors.h>
#include <drivers/pictrl.h>


struct registers {
    unsigned int ds;                                     // Pushed manually
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    unsigned int int_no, err_code;                       // Pushed by the macro
    unsigned int eip, cs, eflags, useresp, ss;           // Pushed by the CPU
};

void irq_handler(struct registers r) {

    if (r.int_no == 33) {
        printk(CYAN, "key pressed!\n");
    }

    pic_send_eoi(r.int_no - 32);
}

void exception_handler(struct registers r) {
    k_panic(__FILE__, __LINE__, "Exception HANDLER");

    for(;;);
}
