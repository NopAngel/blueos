// arch/idt.c
#include "idt.h"
#include "ports.h"
#include <include/printk.h>
extern void syscall_isr_wrapper(void);

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));


struct idt_ptr idtp;
struct idt_entry idt[256];

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;


    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }


    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);  // IRQ0
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);  // IRQ1 - keyboard
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);  // IRQ2
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);  // IRQ3
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);  // IRQ4
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);  // IRQ5
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);  // IRQ6
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);  // IRQ7
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);  // IRQ8
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);  // IRQ9
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E); // IRQ10
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E); // IRQ11
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E); // IRQ12
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E); // IRQ13
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E); // IRQ14
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E); // IRQ15
    idt_set_gate(128, (uint32_t)syscall_isr_wrapper, 0x08, 0xEE);
    
    idt_load((uint32_t)&idtp);
    printk(WHITE, "IDT: IRQs y Syscalls (0x80) cargadas\n");
}
