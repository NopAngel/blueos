#ifndef _BLUEOS_ARCH_H
#define _BLUEOS_ARCH_H


void arch_init(void);
void arch_idle(void);
void arch_enable_interrupts(void);
void arch_disable_interrupts(void);
void arch_putc(char c); 

#endif