#include <drivers/bcma.h>
#include <blueos/io.h>
#include <blueos/printk.h>
#include <drivers/bcma.h>
#include <blueos/colors.h>

#define BCMA_ENUM_BASE    0x18000000 
#define BCMA_CORE_SIZE    0x1000

void bcma_scan_bus() {
    printk(WHITE, "[  BUS  ] BCMA: Scanning Broadcom Specific AMBA bus...\n");

    uint32_t chip_id = bcma_read32(BCMA_CC_ID); 
    
    if (chip_id == 0xFFFFFFFF || chip_id == 0) {
        printk(YELLOW, "BCMA: No Broadcom bus found on this interface.\n");
        return;
    }

    printk(GREEN, "[  OK  ] BCMA: Found Chip ID: 0x%x\n", chip_id);

    bcma_init_core(BCMA_CORE_WIRELESS);
}

void bcma_init_core(uint16_t core_id) {
    switch(core_id) {
        case BCMA_CORE_WIRELESS:
            printk(CYAN, "BCMA: Initializing Wireless LAN core (802.11)...\n");

            break;
        case BCMA_CORE_CHIPCOMMON:
            printk(CYAN, "BCMA: ChipCommon core ready (Power Management).\n");
            break;
    }
}