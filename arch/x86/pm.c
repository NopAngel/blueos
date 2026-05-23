#include <stdint.h>

uint64_t gdt2[] __attribute__((aligned(16))) = {
    0x0000000000000000, // Null Descriptor
    0x00cf9a000000ffff, // Code Segment (Selector 0x08)
    0x00cf92000000ffff  // Data Segment (Selector 0x10)
};

struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr;


__attribute__((section(".setup"), code16))
void go_to_protected_mode(uint32_t kernel_entry) {

    gdt_ptr.limit = sizeof(gdt2) - 1;
    gdt_ptr.base  = (uint32_t)gdt2;


    __asm__ __volatile__ (
        "cli\n"
        "lgdtl %0\n"
        "movl %%cr0, %%eax\n"
        "orl $1, %%eax\n"
        "movl %%eax, %%cr0\n"
        "pushl $0x08\n"
        "pushl %1\n"
        "ljmp $0x08, $1f\n"
        "1:\n"
        ".code32\n"
        "movw $0x10, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%ss\n"
        "movl $0x90000, %%esp\n"
        "jmpl *%1\n"
        :
        : "m"(gdt_ptr), "r"(kernel_entry)
        : "eax"
    );
}
