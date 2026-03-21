#include <stdint.h>

#define GPIO_BASE_RISCV 0x10012000
#define GPIO_BASE_I386  0x03F8 

void gpio_write(int pin, int val) {
#if defined(__riscv)
    volatile uint32_t *gpio_data = (uint32_t *)GPIO_BASE_RISCV;
    if (val) *gpio_data |= (1 << pin);
    else     *gpio_data &= ~(1 << pin);
#elif defined(__i386__)
    // asm volatile("outb %0, %1" : : "a"((uint8_t)val), "Nd"((uint16_t)GPIO_BASE_I386));
#endif
}

int gpio_read(int pin) {
#if defined(__riscv)
    volatile uint32_t *gpio_data = (uint32_t *)GPIO_BASE_RISCV;
    return (*gpio_data >> pin) & 1;
#elif defined(__i386__)
    return 0; // Mock by i386
#endif
}