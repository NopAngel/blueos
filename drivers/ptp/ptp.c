#include <drivers/ptp.h>
#include <kernel/printk.h>


void ptp_init() {
    volatile uint32_t *ctl = (uint32_t *)PTP_TS_CTL;
    
    *ctl |= 0x01; 
    
    printk(CYAN, "[PTP] Precision Time Protocol Clock initialized.\n");
}

void ptp_get_time(struct blueos_timespec *ts) {
    volatile uint32_t *ts_low = (uint32_t *)PTP_TS_LOW;
    volatile uint32_t *ts_high = (uint32_t *)PTP_TS_HIGH;

    ts->tv_sec = *ts_high;
    ts->tv_nsec = *ts_low;
}

void ptp_adj_freq(int32_t ppb) {
    volatile uint32_t *addend = (uint32_t *)PTP_TS_ADDEND;
    
    *addend += (ppb / 10); 
    
    // printk(WHITE, "[PTP] Frequency adjusted by %d ppb\n", ppb);
}