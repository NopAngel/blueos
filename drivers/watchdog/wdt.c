#include <drivers/watchdog.h>
#include <kernel/printk.h>

void watchdog_init(uint32_t timeout_ms) {
    volatile uint32_t *wdt_cfg = (uint32_t *)WDT_CONFIG;
    volatile uint32_t *wdt_rel = (uint32_t *)WDT_RELOAD;

    *wdt_rel = timeout_ms * 1000;

    *wdt_cfg = 0x3; 

    printk(YELLOW, "[WDT] Watchdog armed for %d ms\n", timeout_ms);
}


void watchdog_kick() {
    volatile uint32_t *wdt_feed = (uint32_t *)WDT_FEED;
    *wdt_feed = WDT_FEED_MAGIC;
}

void watchdog_stop() {
    volatile uint32_t *wdt_cfg = (uint32_t *)WDT_CONFIG;
    *wdt_cfg = 0x0;
    printk(WHITE, "[WDT] Watchdog disabled.\n");
}