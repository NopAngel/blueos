#include <kernel/printk.h>
#include <kernel/colors.h>
#include <arch/riscv/plic.h>


int virt_irq_is_pending(unsigned char irq);
void virt_irq_ack(unsigned char irq);

extern void virt_irq_ack(uint8_t irq);
extern int virt_irq_is_pending(uint8_t irq);
extern void virt_irq_unmask(uint8_t irq);

void handle_virtual_device(unsigned char irq) {
    switch(irq) {
        case 10:
            // uart_handler();
            printk(CYAN, "Interrupt: UART activity detected (IRQ 10)\n");
            break;
        case 1:
            printk(YELLOW, "Interrupt: Virtual Device 1\n");
            break;
        default:
            printk(RED, "Interrupt: Unknown IRQ %d\n", irq);
            break;
    }
}


void dispatch_interrupts() {
    uint32_t irq = plic_claim();

    if (irq != 0) {
        handle_virtual_device((unsigned char)irq);

        plic_complete(irq);
    }
}
