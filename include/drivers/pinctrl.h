#ifndef PINCTRL_H
#define PINCTRL_H

void pinctrl_init();
void pinctrl_set_mode(int pin, int is_input);
void pinctrl_write(int pin, int state);

#endif