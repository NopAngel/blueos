#include <drivers/haptics.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

extern int i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data);

static struct hx_device main_hx;


void hx_init(uint8_t addr) {
    main_hx.i2c_addr = addr;
    i2c_write_reg(addr, HX_REG_MODE, 0x00); 
    
    printk(CYAN, "[HX] Haptic Controller initialized at I2C 0x%x\n", addr);
}

void hx_play_effect(uint8_t effect_id) {
    i2c_write_reg(main_hx.i2c_addr, HX_REG_WAVEFORM, effect_id);
    i2c_write_reg(main_hx.i2c_addr, HX_REG_GO, 0x01);
    // printk(WHITE, "[HX] Playing haptic effect %d\n", effect_id);
}