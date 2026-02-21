extern int virt_irq_is_pending(unsigned char irq);
extern void virt_irq_ack(unsigned char irq);
extern void virt_irq_unmask(unsigned char irq);

void handle_virtual_device(unsigned char irq) {
    switch(irq) {
        case 0:
            break;
        case 1:
            break;
        default:
            break;
    }
}

void dispatch_virtual_interrupts() {
    for (int i = 0; i < 16; i++) {
        if (virt_irq_is_pending(i)) {
            handle_virtual_device(i);
            virt_irq_ack(i);
        }
    }
}