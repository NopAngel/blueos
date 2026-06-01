#include <kernel/io.h>
#include <kernel/printk.h>
#include <drivers/bcma.h>
#include <kernel/colors.h>

#define BCMA_ENUM_BASE    0x18000000
#define BCMA_CORE_SIZE    0x1000

void bcma_scan_bus() {
    boot_msg("BUS", "BCMA: Scanning Broadcom Specific AMBA bus...\n", 1);

    uint32_t chip_id = bcma_read32(BCMA_CC_ID);

    if (chip_id == 0xFFFFFFFF || chip_id == 0) {
        boot_msg("BCMA", "No Broadcom bus found on this interface.\n", 1);
        return;
    }

    boot_msg("BCMA", "Found Chip ID\n", 0);

    bcma_init_core(BCMA_CORE_WIRELESS);
}

void bcma_init_core(uint16_t core_id) {
    switch(core_id) {
        case BCMA_CORE_WIRELESS:
            boot_msg("BCMA", "Initializing Wireless LAN core (802.11)...\n", 0);


            break;
        case BCMA_CORE_CHIPCOMMON:
            boot_msg("BCMA", "ChipCommon core ready (Power Management)\n", 0);

            break;
    }
}
