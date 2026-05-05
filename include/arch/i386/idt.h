#ifndef _BLUEOS_IDT_H
#define _BLUEOS_IDT_H

#include <stdint.h>

#define IDT_TA_INTERRUPT  0x8E
#define IDT_TA_TRAP       0x8F
#define IDT_TA_CALL       0xEE


struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

struct idt_entry {
    uint16_t base_low;    // Offset 0-15
    uint16_t selector;    // (GDT)
    uint8_t  zero;        // always 0
    uint8_t  flags;       // (P, DPL, DT, Type)
    uint16_t base_high;   // Offset 16-31
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));


void idt_init(void);

void idt_set_gate(uint8_t n, uint32_t base, uint16_t sel, uint8_t flags);
void idt_load(void);

#endif
