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
    printk(YELLOW, "[ PINCTRL ] Initializing GPIO Controller (%s)...\n", 
#ifdef x86
    "Port I/O"
#else
    "MMIO"
#endif
    );

    gpio_write_reg(0x00, 0xFFFFFFFF); // GPIO_USE_SEL
    unsigned int val = gpio_read_reg(0x00);
    
    if (val != 0) {
        printk(GREEN, "[  OK  ] Pinctrl: GPIO Controller ready at 0x%x\n", GPIO_BASE_ADDR);
    } else {
        printk(RED, "[ ERR  ] Pinctrl: Controller not found.\n");
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
    printk(CYAN, "[ PINCTRL ] Pin %d set to %s\n", pin, state ? "HIGH" : "LOW");
}