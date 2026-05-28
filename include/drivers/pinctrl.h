#ifndef PINCTRL_H
#define PINCTRL_H

#ifdef x86
    #define GPIO_BASE_ADDR 0x0500
#elif defined(RISCV)
    #define GPIO_BASE_ADDR 0x10002000 
#endif

void pinctrl_init();
void pinctrl_set_mode(int pin, int is_input);
void pinctrl_write(int pin, int state);

#endif