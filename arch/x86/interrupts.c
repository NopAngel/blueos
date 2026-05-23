#include <stdint.h>

extern void virt_irq_ack(uint8_t irq);
extern int virt_irq_is_pending(uint8_t irq);
extern void virt_irq_unmask(uint8_t irq);

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
