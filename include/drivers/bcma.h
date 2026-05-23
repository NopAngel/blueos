#ifndef BCMA_H
#define BCMA_H

#include <stdint.h>

#define BCMA_ENUM_BASE    0x18000000 
#define BCMA_CORE_SIZE    0x1000

#define BCMA_CC_ID            0x0000  
#define BCMA_CORE_CHIPCOMMON  0x800
#define BCMA_CORE_WIRELESS    0x812
#define BCMA_CORE_PCIE        0x820

void bcma_scan_bus();
void bcma_init_core(uint16_t core_id);


static inline uint32_t bcma_read32(uint32_t reg) {
    return *(volatile uint32_t*)(BCMA_ENUM_BASE + reg);
}

#endif