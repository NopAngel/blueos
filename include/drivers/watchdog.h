#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>

/* Watchdog Control Registers (Base address varies by board) */
#define WDT_BASE        0x10007000 
#define WDT_CONFIG      (WDT_BASE + 0x00)
#define WDT_RELOAD      (WDT_BASE + 0x04)
#define WDT_FEED        (WDT_BASE + 0x08)

/* Magic key to reset the counter (Standard for many WDTs) */
#define WDT_FEED_MAGIC  0x55AAAA55

void watchdog_init(uint32_t timeout_ms);
void watchdog_kick();
void watchdog_stop();

#endif 