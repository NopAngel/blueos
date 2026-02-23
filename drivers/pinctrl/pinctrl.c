#include <blueos/ports.h>
#include <blueos/printk.h>
#include <blueos/colors.h>

#define GPIO_BASE_ADDR 0x0500 
#define GPIO_USE_SEL   (GPIO_BASE_ADDR + 0x00) 
#define GPIO_IO_SEL    (GPIO_BASE_ADDR + 0x04) 
#define GPIO_LVL       (GPIO_BASE_ADDR + 0x0C)


void pinctrl_init() {
    printk(YELLOW, "[ PINCTRL ] Initializing Chipset GPIO Controller...\n");
    outl(GPIO_USE_SEL, 0xFFFFFFFF);
    unsigned int val = inl(GPIO_USE_SEL);
    
    if (val == 0xFFFFFFFF || val != 0) {
        printk(GREEN, "[  OK  ] Pinctrl: GPIO Controller ready at 0x%x\n", GPIO_BASE_ADDR);
    } else {
        printk(RED, "[ ERR  ] Pinctrl: Controller not found.\n");
    }
}


void pinctrl_set_mode(int pin, int is_input) {
    unsigned int current = inl(GPIO_IO_SEL);
    if (is_input) {
        current |= (1 << pin);
    } else {
        current &= ~(1 << pin);
    }
    outl(GPIO_IO_SEL, current);
}


void pinctrl_write(int pin, int state) {
    unsigned int current = inl(GPIO_LVL);
    if (state) {
        current |= (1 << pin);
    } else {
        current &= ~(1 << pin);
    }
    outl(GPIO_LVL, current);
    printk(CYAN, "[ PINCTRL ] Pin %d set to %s\n", pin, state ? "HIGH" : "LOW");
}