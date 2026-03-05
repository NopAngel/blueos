#ifndef BCMA_H
#define BCMA_H

#include <stdint.h>

/* Direcciones base para Broadcom Specific AMBA */
#define BCMA_ENUM_BASE    0x18000000 
#define BCMA_CORE_SIZE    0x1000

/* IDs de Registros y Cores */
#define BCMA_CC_ID            0x0000  
#define BCMA_CORE_CHIPCOMMON  0x800
#define BCMA_CORE_WIRELESS    0x812
#define BCMA_CORE_PCIE        0x820

void bcma_scan_bus();
void bcma_init_core(uint16_t core_id);

/**
 * bcma_read32 - Lee un registro de 32 bits del bus BCMA
 * Usamos un puntero volátil para que el compilador no optimice
 * la lectura, ya que el valor puede cambiar por hardware.
 */
static inline uint32_t bcma_read32(uint32_t reg) {
    return *(volatile uint32_t*)(BCMA_ENUM_BASE + reg);
}

#endif