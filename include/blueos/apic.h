#ifndef _BLUEOS_APIC_H
#define _BLUEOS_APIC_H

#include <stdint.h>

#define APIC_BASE_ADDR      0xFEE00000
#define APIC_ID             0x0020  
#define APIC_VER            0x0030 
#define APIC_TPR            0x0080  
#define APIC_EOI            0x00B0  
#define APIC_LDR            0x00D0  
#define APIC_SVR            0x00F0  
#define APIC_ICR_LOW        0x0300  
#define APIC_ICR_HIGH       0x0310  
#define APIC_LVT_TIMER      0x0320 

void apic_init(void);
void apic_eoi(void);
uint32_t apic_read(uint32_t reg);
void apic_write(uint32_t reg, uint32_t data);

#endif