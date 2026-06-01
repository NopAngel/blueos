#include <kernel/hal.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <drivers/pinctrl.h>
#ifdef x86
    #include <kernel/ports.h>
#endif



static inline void gpio_write_reg(unsigned short offset, unsigned int val) {
#ifdef x86
    outl(GPIO_BASE_ADDR + offset, val);
#else
    volatile unsigned int *reg = (volatile unsigned int *)(GPIO_BASE_ADDR + offset);
    *reg = val;
#endif
}

static inline unsigned int gpio_read_reg(unsigned short offset) {
#ifdef x86
    return inl(GPIO_BASE_ADDR + offset);
#else
    volatile unsigned int *reg = (volatile unsigned int *)(GPIO_BASE_ADDR + offset);
    return *reg;
#endif
}

void pinctrl_init() {
    printk("PINCTRL", "Initializing GPIO Controller...\n", 0);

    gpio_write_reg(0x00, 0xFFFFFFFF); // GPIO_USE_SEL
    unsigned int val = gpio_read_reg(0x00);

    if (val != 0) {
        printk("PINCTRL", "GPIO Controller ready\n", 0);
    } else {
        printk("PINCTRL", "Controller not found.\n", 2);
    }
}

void pinctrl_set_mode(int pin, int is_input) {
    unsigned int current = gpio_read_reg(0x04); // GPIO_IO_SEL
    if (is_input) {
        current |= (1 << pin);
    } else {
        current &= ~(1 << pin);
    }
    gpio_write_reg(0x04, current);
}

void pinctrl_write(int pin, int state) {
    unsigned int current = gpio_read_reg(0x0C); // GPIO_LVL
    if (state) {
        current |= (1 << pin);
    } else {
        current &= ~(1 << pin);
    }
    gpio_write_reg(0x0C, current);
}
