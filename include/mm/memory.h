
#ifndef MEMORY_H
#define MEMORY_H


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef int size_t;


typedef int bool;
#define true 1
#define false 0


#define PAGE_SIZE 4096           
#define BITMAP_SIZE 8192        
#define KERNEL_START 0x100000    
#define KERNEL_SIZE 0x100000    

struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
};

struct memory_map_entry {
    uint32_t size;
    uint32_t base_low;
    uint32_t base_high;
    uint32_t length_low;
    uint32_t length_high;
    uint32_t type;
};

struct memory_manager {
    uint32_t total_memory;
    uint32_t free_memory;
    uint32_t used_memory;
    uint32_t total_pages;
    uint32_t free_pages;
    uint32_t next_free;
    uint8_t* bitmap;
    uint32_t bitmap_size;
    int initialized;
};


void mm_init(struct multiboot_info* mbi);
void* kmalloc(uint32_t size);
void kfree(void* ptr, uint32_t size);
void* kcalloc(uint32_t num, uint32_t size);
uint32_t mm_get_total(void);
uint32_t mm_get_free(void);
uint32_t mm_get_used(void);
void mm_dump_info(void);

void mm_memset(void* ptr, uint8_t value, uint32_t size);
void mm_memcpy(void* dest, const void* src, uint32_t size);
void mm_memmove(void* dest, const void* src, uint32_t size);

#endif
