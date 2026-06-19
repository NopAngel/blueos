#include <drivers/uio.h>
#include <kernel/printk.h>
#include <mm/vmm.h>

int uio_register_device(struct uio_info *info) {
  printk("[UIO] Registering device '%s' at 0x%lx\n", info->name, info->addr);

  if (info->irq > 0) {
    // irq_register_handler(info->irq, uio_interrupt_handler);
  }

  return 0;
}

void uio_interrupt_handler(int irq) {
  // wakeup_process_waiting_on_uio(irq);
}
