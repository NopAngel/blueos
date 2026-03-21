#include <drivers/w1.h>

extern void delay_us(uint32_t us);
extern void gpio_write(int pin, int val);
extern int  gpio_read(int pin);

uint8_t w1_reset(int pin) {
    uint8_t presence = 0;
    
    gpio_write(pin, 0); // Pull low
    delay_us(W1_RESET_US);
    gpio_write(pin, 1); // Release
    delay_us(70);          
    
    presence = !gpio_read(pin); 
    delay_us(410);
    
    return presence;
}

void w1_write_bit(int pin, uint8_t bit) {
    if (bit) {
        gpio_write(pin, 0);
        delay_us(W1_WRITE1_US);
        gpio_write(pin, 1);
        delay_us(55);
    } else {
        gpio_write(pin, 0);
        delay_us(W1_WRITE0_US);
        gpio_write(pin, 1);
        delay_us(5);
    }
}

uint8_t w1_read_byte(int pin) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        gpio_write(pin, 0);
        delay_us(3);
        gpio_write(pin, 1);
        delay_us(10);
        if (gpio_read(pin)) byte |= (1 << i);
        delay_us(50);
    }
    return byte;
}