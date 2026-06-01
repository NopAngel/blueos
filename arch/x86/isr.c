#include <arch/x86/idt.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <kernel/colors.h>

extern void vmm_handle_cow(uint32_t addr, uint32_t *pte);
extern void vmm_handle_demand_paging(uint32_t addr, uint32_t *pte);

void isr_handler(struct trap_frame *regs) {
    if (regs->interrupt_no == 14) { // Page Fault
        uint32_t fault_addr;
        __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));

        printk("PAGE FAULT: addr=0x%08x err=0x%08x\n", fault_addr, regs->error_code);

        // uint32_t *pte = vmm_get_pte(fault_addr);
        uint32_t *pte = 0; // Placeholder

        int present = regs->error_code & 0x1;
        int write   = regs->error_code & 0x2;

        /* Lógica de Copy-on-Write */
        if (present && write && pte && (*pte & 0x200)) {
            vmm_handle_cow(fault_addr, pte);
            return;
        }

        if (!present) {
            // vmm_handle_demand_paging(fault_addr, pte);
            // return;
        }
    } else {
        printk("EXCEPTION %u: error=0x%08x\n", regs->interrupt_no, regs->error_code);
    }

    k_panic(0, "ISR HANDLER ERR");
    for(;;);
}
