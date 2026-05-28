#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define HIBERNATE_PARTITION 1
#define RAM_START 0x80000000  
#define RAM_SIZE  (128 * 1024 * 1024) 


int disk_write(uint32_t partition, uint32_t ram_addr, uint32_t size);

#endif