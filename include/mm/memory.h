
#ifndef MEMORY_H
#define MEMORY_H
#include <kernel/types.h>
#include <stdbool.h>
#include <stdint.h>


#define true 1
#define false 0


#define PAGE_SIZE 4096
#define BITMAP_SIZE 8192
#define KERNEL_START 0x100000
#define KERNEL_SIZE 0x100000


struct memory_manager {
    uint8_t* bitmap;
    uintptr_t total_memory;
    uint32_t  total_pages;
    uint32_t  bitmap_size;
    uint32_t  next_free;
    int       initialized;
};

struct memory_map_entry {
    uint32_t size;
    uint32_t base_low;
    uint32_t base_high;
    uint32_t length_low;
    uint32_t length_high;
    uint32_t type;
};

extern struct memory_manager mm;

static void set_bit(uint32_t bit, int value);

static int get_bit(uint32_t bit);
void mm_init_universal(uintptr_t ram_start, uint64_t ram_size);
void* kmalloc(uint32_t size);
void* mm_memset(void* s, int c, size_t n);
void* mm_memcpy(void* dest, const void* src, size_t n);
#if defined(x86)
void x86_memory_prepare();
void mm_init();
uint32_t mm_get_total();
#endif


#endif
