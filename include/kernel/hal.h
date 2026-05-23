#ifndef HAL_H
#define HAL_H

#include <stdint.h>

void sbi_system_reset(uint32_t type, uint32_t reason);

void arch_cpu_halt();

int fat16_read_file(const char* path, char* buffer);
#endif
