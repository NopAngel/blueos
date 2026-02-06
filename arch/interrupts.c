extern int virt_irq_is_pending(unsigned char irq);
extern void virt_irq_ack(unsigned char irq);
extern void virt_irq_unmask(unsigned char irq);

void handle_virtual_device(unsigned char irq) {
    switch(irq) {
        case 0:
            // Ejemplo: printk("VIRT: Timer tick", ...);
            break;
        case 1:
            // Ejemplo: printk("VIRT: Keyboard event", ...);
            break;
        default:
            // printk("VIRT: Unknown IRQ", ...);
            break;
    }
}

void dispatch_virtual_interrupts() {
    for (int i = 0; i < 16; i++) {
        if (virt_irq_is_pending(i)) {
            // printk("VIRT: Procesando interrupción virtual %d", i, ...);

            handle_virtual_device(i);
            virt_irq_ack(i);
        }
    }
}