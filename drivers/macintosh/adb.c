#include <drivers/adb.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

#define MAC_VIA_BASE    0x50F00000 
#define VIA_REG_DATA    (MAC_VIA_BASE + 0x00)


void mac_adb_init() {
    printk(CYAN, "[MAC] Initializing Apple Desktop Bus (ADB)...\n");
    // gpio_write(ADB_PIN, 0);
    // delay_us(800);
}

int mac_adb_talk(uint8_t addr, uint8_t reg, uint8_t *buffer) {
    uint8_t command = (addr << 4) | ADB_CMD_TALK | reg;
    
    printk(WHITE, "[MAC] ADB Talk to device 0x%x\n", addr);
    return 0; 
}