#ifndef IO_H
#define IO_H

#include <stdint.h>


#if defined(__riscv)
    #define outb(addr, val) (*(volatile uint8_t *)(addr) = (val))
    #define inb(addr)       (*(volatile uint8_t *)(addr))
#elif defined(__i386__)
        
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outl(uint16_t port, uint32_t val) {
    asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline void insw(uint16_t port, void *addr, uint32_t count) {
    asm volatile("cld; rep insw" :
                 "+D"(addr), "+c"(count) :
                 "d"(port) :
                 "memory");
}

#endif  

#endif