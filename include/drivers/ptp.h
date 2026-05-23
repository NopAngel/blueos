#ifndef PTP_H
#define PTP_H

#include <stdint.h>

#define PTP_BASE          0x3000A000
#define PTP_TS_LOW        (PTP_BASE + 0x00) 
#define PTP_TS_HIGH       (PTP_BASE + 0x04) 
#define PTP_TS_ADDEND     (PTP_BASE + 0x08) 
#define PTP_TS_CTL        (PTP_BASE + 0x0C) 

struct blueos_timespec {
    uint64_t tv_sec;
    uint32_t tv_nsec;
};

void ptp_init();
void ptp_get_time(struct blueos_timespec *ts);
void ptp_set_time(struct blueos_timespec *ts);
void ptp_adj_freq(int32_t ppb); 

#endif