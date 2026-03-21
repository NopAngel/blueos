#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define PAGE_SIZE 4096

struct memory_manager {
    uint8_t* bitmap;
    uintptr_t total_memory;
    uint32_t  total_pages;
    uint32_t  bitmap_size;
    uint32_t  next_free;
    int       initialized;
};

void mm_init(uint32_t mem_size_mb);

void* kmalloc(uint32_t size);
void  mm_dump_info(void);

#endif