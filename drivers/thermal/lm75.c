#include <stdint.h>
#include <kernel/io.h>
#include <drivers/i2c.h>

extern uint16_t g_smbus_base;

#define LM75_ADDR 0x48 

int32_t thermal_get_temp() {
    if (g_smbus_base == 0) return -999; 
    outb(g_smbus_base + 0x04, (LM75_ADDR << 1) | 1);
    outb(g_smbus_base + 0x03, 0x00);

    outb(g_smbus_base + 0x02, 0x48);
    int timeout = 1000;
    while (!(inb(g_smbus_base + 0x00) & 0x02) && --timeout > 0);

    int8_t temp = (int8_t)inb(g_smbus_base + 0x05);
    
    return (int32_t)temp;
}