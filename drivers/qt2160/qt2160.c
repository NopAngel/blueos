#include <config.h>
#include <drivers/keyboard.h>
#include <drivers/i2c.h>
#include <kernel/printk.h>

/* QT2160 Register Map */
#define QT2160_CHIP_ID       0x00
#define QT2160_STAT          0x02 // Status register (detects key press)
#define QT2160_KEYS_LSB      0x03 // Key status LSB
#define QT2160_KEYS_MSB      0x04 // Key status MSB
#define QT2160_LP            0x0B // Low power configuration
#define QT2160_I2C_ADDR 0x0D // Default 7-bit address

extern void keyboard_handler_raw(uint8_t code);

/**
 * qt2160_init - Initializes the touch slider/keyboard controller
 */
void qt2160_init() {
    uint8_t chip_id = 0;
    
    /* 1. Verify Chip ID */
    if (i2c_read_reg(QT2160_I2C_ADDR, QT2160_CHIP_ID, &chip_id) != 0) {
        pr_err("QT2160: Failed to communicate via I2C\n");
        return;
    }

    if (chip_id != 0x11) { // 0x11 is the standard ID for QT2160
        pr_err("QT2160: Invalid Chip ID (0x%x)\n", chip_id);
        return;
    }

    /* 2. Configure Low Power mode or Burst length if needed */
    i2c_write_reg(QT2160_I2C_ADDR, QT2160_LP, 0x01); // Example: Wake up

    pr_info("QT2160: Touch controller initialized successfully.\n");
}

/**
 * qt2160_poll - Polls the device for new key events
 * In a real OS, this would be triggered by a GPIO interrupt pin.
 */
void qt2160_poll() {
    uint8_t status;
    uint16_t key_matrix;
    uint8_t lsb, msb;

    /* 1. Check if there is a change in status */
    if (i2c_read_reg(QT2160_I2C_ADDR, QT2160_STAT, &status) != 0) return;

    if (status & 0x01) { // Bit 0 indicates a key press change
        /* 2. Read which keys are pressed (16 bits total) */
        i2c_read_reg(QT2160_I2C_ADDR, QT2160_KEYS_LSB, &lsb);
        i2c_read_reg(QT2160_I2C_ADDR, QT2160_KEYS_MSB, &msb);
        
        key_matrix = (msb << 8) | lsb;

        /* 3. Convert matrix bit to scancode and send to handler */
        for (int i = 0; i < 16; i++) {
            if (key_matrix & (1 << i)) {
                keyboard_handler_raw(i + 0x50);
            }
        }
    }
}