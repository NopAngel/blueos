// arch/ports.h
#ifndef PORTS_H
#define PORTS_H

#include "../include/types.h"


static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

static inline uint16_t inw(uint16_t port) {
    uint16_t data;
    __asm__ volatile("inw %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

static inline void outw(uint16_t port, uint16_t data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t data;
    __asm__ volatile("inl %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

static inline void outl(uint16_t port, uint32_t data) {
    __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}


static inline uint8_t read_cmos(uint8_t reg) {
    outb(0x70, reg);
    io_wait();
    return inb(0x71);
}

static inline void write_cmos(uint8_t reg, uint8_t data) {
    outb(0x70, reg);
    io_wait();
    outb(0x71, data);
}

static inline void enable_interrupts(void) {
    __asm__ volatile("sti");
}

static inline void disable_interrupts(void) {
    __asm__ volatile("cli");
}


static inline uint32_t get_cr0(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    return cr0;
}

static inline void set_cr0(uint32_t cr0) {
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

static inline uint32_t get_cr2(void) {
    uint32_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}

static inline uint32_t get_cr3(void) {
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static inline void set_cr3(uint32_t cr3) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(cr3));
}

static inline uint32_t get_cr4(void) {
    uint32_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}

static inline void set_cr4(uint32_t cr4) {
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));
}

static inline void invlpg(void* addr) {
    __asm__ volatile("invlpg (%0)" : : "r"(addr) : "memory");
}

static inline void load_gdt(uint32_t gdt_ptr) {
    __asm__ volatile("lgdt (%0)" : : "r"(gdt_ptr));
}

static inline void load_idt(uint32_t idt_ptr) {
    __asm__ volatile("lidt (%0)" : : "r"(idt_ptr));
}

static inline void reload_segments(void) {
    __asm__ volatile(
        "mov $0x10, %ax\n"
        "mov %ax, %ds\n"
        "mov %ax, %es\n"
        "mov %ax, %fs\n"
        "mov %ax, %gs\n"
        "mov %ax, %ss\n"
        "ljmp $0x08, $1f\n"
        "1:"
    );
}

#endif
