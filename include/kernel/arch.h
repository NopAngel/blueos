#ifndef KERNEL_ARCH_H
#define KERNEL_ARCH_H

void arch_init(void);
void arch_disable_interrupts(void);
void arch_enable_interrupts(void);
void arch_idle(void);

#endif