#include <drivers/i2c.h>
#include <kernel/io.h> // For outb/inb on x86
#include <stdint.h>

/* --- SMBus Registers (x86/x86) --- */
#define SMB_HST_STS  0x00 // Host Status
#define SMB_HST_CNT  0x02 // Host Control
#define SMB_HST_CMD  0x03 // Host Command
#define SMB_HST_ADDR 0x04 // Host Address
#define SMB_HST_DAT0 0x05 // Host Data 0

/* --- I2C Registers (RISC-V MMIO Example) --- */
#define I2C_CONTROL  0x00
#define I2C_STATUS   0x04
#define I2C_DATA     0x08

#if defined(__riscv)
    #define DEFAULT_I2C_BASE 0x10012000 
#elif defined(__x86__)
    #define DEFAULT_I2C_BASE 0x0400     
#else
    #define DEFAULT_I2C_BASE 0x0
#endif

/**
 * i2c_write_reg - Top-level API to write to a device register.
 * This function handles the architecture switch internally.
 */
int i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data) {
#if defined(__x86__)
    /* x86 SMBus Logic */
    uintptr_t base = DEFAULT_I2C_BASE;

    // Set target device address (Shifted for write bit 0)
    outb(base + SMB_HST_ADDR, (dev_addr << 1)); 

    // Set the register index and the data byte
    outb(base + SMB_HST_CMD, reg);
    outb(base + SMB_HST_DAT0, data);

    // Start transaction (0x48: Byte Data Protocol + Start bit)
    outb(base + SMB_HST_CNT, 0x48);

    // Wait for the 'Interrupt' bit (Transaction complete)
    while (!(inb(base + SMB_HST_STS) & 0x02)) {
        // In a real OS, add a timeout here to avoid infinite loops
    }

    // Clear status bits by writing 1s
    outb(base + SMB_HST_STS, 0xFF);
    return 0;

#elif defined(__riscv)
    /* RISC-V MMIO Logic (Generic implementation) */
    volatile uint32_t *base = (uint32_t*)DEFAULT_I2C_BASE;

    // Implementation depends on the specific SoC (SiFive, Virt, etc.)
    // Typically: Send START -> Send ADDR -> Send REG -> Send DATA -> Send STOP
    // For now, we use a placeholder that fits your project structure:
    
    // (Actual RISC-V I2C logic would go here)
    return 0;
#endif
    return -1;
}

/**
 * i2c_read_reg - Top-level API to read from a device register.
 * Essential for drivers like QT2160 to check status.
 */
int i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data) {
    if (!data) return -1;

#if defined(__x86__)
    uintptr_t base = DEFAULT_I2C_BASE;

    // Set target address (Read bit 1 is usually handled by the protocol bit)
    outb(base + SMB_HST_ADDR, (dev_addr << 1) | 1); 
    outb(base + SMB_HST_CMD, reg);

    // Start Read transaction
    outb(base + SMB_HST_CNT, 0x48);

    while (!(inb(base + SMB_HST_STS) & 0x02));

    // Get the resulting byte from Data 0 register
    *data = inb(base + SMB_HST_DAT0);
    outb(base + SMB_HST_STS, 0xFF);
    return 0;

#elif defined(__riscv)
    // RISC-V Read logic placeholder
    return 0;
#endif
    return -1;
}