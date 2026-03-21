#include <drivers/i2c.h>
#include <kernel/io.h> 


void i2c_write_byte(uintptr_t base, uint8_t addr, uint8_t reg, uint8_t data) {
    
    outb(base + SMB_HST_ADDR, (addr << 1)); 

    outb(base + SMB_HST_CMD, reg);
    outb(base + SMB_HST_DAT0, data);

    outb(base + SMB_HST_CNT, 0x48);

    while (!(inb(base + SMB_HST_STS) & 0x02)) {
    }

    outb(base + SMB_HST_STS, 0xFF);
}


#if defined(__riscv)
    #define DEFAULT_I2C_BASE 0x10012000 
#elif defined(__i386__)
    #define DEFAULT_I2C_BASE 0x0400     
#endif


int i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data) {
    i2c_write_byte(DEFAULT_I2C_BASE, dev_addr, reg, data);
    return 0; 
}