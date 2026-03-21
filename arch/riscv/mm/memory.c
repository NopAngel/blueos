/*
 * BlueOS mm/memory_riscv.c - Physical Page Frame Allocator
 * Ported to RISC-V (QEMU Virt Support)
 */

#define MEMORY_C
#include <mm/memory.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <stdint.h>

#define RAM_START         0x80000000  
#define PAGE_SIZE         4096        
#define BITMAP_SIZE       32768       

/* --- Global State --- */
extern uint32_t _start;               
extern uint32_t _end;                 

static struct memory_manager mm = {0};

static uint8_t bitmap_storage[BITMAP_SIZE];

/* Stats */
unsigned int total_memory_kb = 0;

/* --- Core Memory Utilities --- */

void mm_memset(void* ptr, uint8_t value, uint32_t size) {
    uint8_t* p = (uint8_t*)ptr;
    for (uint32_t i = 0; i < size; i++) p[i] = value;
}

static void set_bit(uint32_t bit, int value) {
    uint32_t byte = bit / 8;
    uint32_t bit_in_byte = bit % 8;
    if (value) mm.bitmap[byte] |= (1 << bit_in_byte);
    else mm.bitmap[byte] &= ~(1 << bit_in_byte);
}

static int get_bit(uint32_t bit) {
    uint32_t byte = bit / 8;
    uint32_t bit_in_byte = bit % 8;
    return (mm.bitmap[byte] >> bit_in_byte) & 1;
}

void mm_init(uint32_t mem_size_mb) {
    if (mm.initialized) return;

    mm.total_memory = (uintptr_t)mem_size_mb * 1024 * 1024;
    mm.total_pages = mm.total_memory / PAGE_SIZE;
    mm.bitmap = bitmap_storage;
    mm.bitmap_size = (mm.total_pages + 7) / 8;

    mm_memset(mm.bitmap, 0, BITMAP_SIZE);

    uintptr_t kernel_start = (uintptr_t)&_start;
    uint32_t reserved_low_pages = (kernel_start - RAM_START) / PAGE_SIZE;
    for (uint32_t i = 0; i < reserved_low_pages; i++) set_bit(i, 1);

    uintptr_t kernel_end = (uintptr_t)&_end;
    uint32_t k_start_page = (kernel_start - RAM_START) / PAGE_SIZE;
    uint32_t k_pages = (kernel_end - kernel_start + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint32_t i = 0; i < k_pages; i++) {
        set_bit(k_start_page + i, 1);
    }

    mm.next_free = k_start_page + k_pages;
    mm.initialized = 1;

    printk(GREEN, "PMM: RISC-V Memory Manager Initialized.\n");
    printk(GRAY, " RAM Start: 0x%lx | Size: %d MB\n", RAM_START, mem_size_mb);
}



/* --- Status Functions --- */

uint32_t mm_get_used(void) {
    uint32_t used = 0;
    for (uint32_t i = 0; i < mm.total_pages; i++) {
        if (get_bit(i)) used++;
    }
    return used * PAGE_SIZE;
}

void mm_dump_info(void) {
    uint32_t used = mm_get_used() / 1024;
    uint32_t total = mm.total_memory / 1024;
    printk(WHITE, "\n[ RISC-V MEMORY REPORT ]\n");
    printk(GRAY, " Total: %d KB | Used: %d KB | Free: %d KB\n", total, used, total - used);
    printk(WHITE, "------------------------\n");
}

