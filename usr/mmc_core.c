#include <kernel/printk.h>

int mmc_init_host() {
    printk("[  MMC  ] Initializing SDHC/MMC Host Controller...\n");
    return 0;
}

void mmc_rescan_bus() {
    printk("MMC: Scanning bus for new cards...\n");
}
