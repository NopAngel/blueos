#ifndef I2C_H
#define I2C_H

#include <stdint.h>

#define SMB_HST_STS   0x00
#define SMB_HST_CNT   0x02
#define SMB_HST_CMD   0x03
#define SMB_HST_ADDR  0x04
#define SMB_HST_DAT0  0x05

int i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data);
int i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data);

#endif