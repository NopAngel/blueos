#include <arch/x86/idt.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <kernel/colors.h>
#include <kernel/timer.h>
#include <drivers/pictrl.h>

void irq_handler(struct trap_frame *r) {

    if (r->interrupt_no == 32) {
        timer_handler();
    } else if (r->interrupt_no == 33) {

    }

    pic_send_eoi(r->interrupt_no - 32);
}

void exception_handler(struct registers r) {
    k_panic(0, "Exception HANDLER");

    for(;;);
}
