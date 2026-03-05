/*
 * BlueOS mm/memory.c - Physical Page Frame Allocator
 *
 * This module manages physical RAM using a bitset (bitmap). 
 * Each bit represents a 4KB page of physical memory.
 *
 * Copyright (C) 2024-2026 NopAngel
 */

#define MEMORY_C
#include <mm/memory.h>
#include <blueos/printk.h>
#include <blueos/colors.h>

/* --- Configuration Macros --- */
#define KERNEL_START_ADDR 0x100000  // 1MB mark
#define PAGE_SIZE         4096      // 4KB Standard Page

/* --- Global State --- */
extern uint32_t _end;              // Defined by linker script
static struct memory_manager mm = {0};
static uint8_t bitmap_storage[BITMAP_SIZE];

uint32_t total_blocks = 0;  
uint32_t used_blocks = 0;

/* Stats for the rest of the system */
unsigned int total_memory_kb = 0;
unsigned int used_memory_kb = 0;

/* --- Core Memory Utilities (Linux style) --- */

void mm_memset(void* ptr, uint8_t value, uint32_t size) {
    uint8_t* p = (uint8_t*)ptr;
    for (uint32_t i = 0; i < size; i++) p[i] = value;
}

void mm_memcpy(void* dest, const void* src, uint32_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (uint32_t i = 0; i < size; i++) d[i] = s[i];
}

/* --- Bitmap Internal Operations --- */

static void set_bit(uint32_t bit, int value) {
    uint32_t byte = bit / 8;
    uint32_t bit_in_byte = bit % 8;

    if (value) 
        mm.bitmap[byte] |= (1 << bit_in_byte);
    else 
        mm.bitmap[byte] &= ~(1 << bit_in_byte);
}

static int get_bit(uint32_t bit) {
    uint32_t byte = bit / 8;
    uint32_t bit_in_byte = bit % 8;
    return (mm.bitmap[byte] >> bit_in_byte) & 1;
}

/* --- Exported API for init_fnc.c --- */

int pmm_test_bit(uint32_t page) {
    if (page >= mm.total_pages) return 1; // Out of bounds is "used"
    return get_bit(page);
}

void pmm_set_bit(uint32_t page) {
    if (page < mm.total_pages) set_bit(page, 1);
}

/**
 * calculate_memory - Probes Multiboot for RAM size
 */
static uint32_t calculate_memory(struct multiboot_info* mbi) {
    // If MBI is null or flags don't have bit 0 (mem info), fallback to 128MB
    if (!mbi || !(mbi->flags & 0x01)) {
        total_memory_kb = 131072; 
        return 128 * 1024 * 1024;
    }

    // Upper memory is in KB, starts at 1MB
    uint32_t mem_kb = mbi->mem_upper + 1024; 
    total_memory_kb = mem_kb;
    return mem_kb * 1024;
}

/**
 * mm_init - Initializes the Physical Memory Manager
 */
void mm_init(struct multiboot_info* mbi) {
    if (mm.initialized) return;

    mm.total_memory = calculate_memory(mbi);
    mm.total_pages = mm.total_memory / PAGE_SIZE;

    mm.bitmap = bitmap_storage;
    mm.bitmap_size = (mm.total_pages + 7) / 8;

    // Safety: Don't overflow our static bitmap storage
    if (mm.bitmap_size > BITMAP_SIZE) {
        mm.bitmap_size = BITMAP_SIZE;
        mm.total_pages = BITMAP_SIZE * 8;
        mm.total_memory = mm.total_pages * PAGE_SIZE;
    }

    // 1. Clear the entire bitmap (Mark all as FREE)
    mm_memset(mm.bitmap, 0, mm.bitmap_size);

    // 2. Protect Low Memory (0 - 1MB) - BIOS, VGA, IVT live here
    uint32_t low_mem_pages = 0x100000 / PAGE_SIZE;
    for (uint32_t i = 0; i < low_mem_pages; i++) {
        set_bit(i, 1);
    }

    // 3. Protect Kernel Image
    uint32_t k_start_page = KERNEL_START_ADDR / PAGE_SIZE;
    uint32_t k_end_addr = (uint32_t)&_end;
    uint32_t k_pages = (k_end_addr - KERNEL_START_ADDR + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint32_t i = 0; i < k_pages; i++) {
        set_bit(k_start_page + i, 1);
    }

    // 4. Update stats
    mm.next_free = k_start_page + k_pages;
    mm.used_memory = (uint32_t)mm_get_used(); // Calculated by counting bits if needed
    mm.free_memory = mm.total_memory - mm.used_memory;
    mm.free_pages = mm.free_memory / PAGE_SIZE;

    mm.initialized = 1;
}

/* --- Allocation Engine --- */

static int find_free_pages(uint32_t count, uint32_t* result) {
    uint32_t consecutive = 0;
    
    for (uint32_t i = mm.next_free; i < mm.total_pages; i++) {
        if (!get_bit(i)) {
            if (++consecutive == count) {
                *result = i - count + 1;
                return 1;
            }
        } else {
            consecutive = 0;
        }
    }
    return 0; // Out of memory
}

void* kmalloc(uint32_t size) {
    if (!mm.initialized || size == 0) return 0;

    uint32_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t start_page;

    if (!find_free_pages(pages_needed, &start_page)) {
        return 0; 
    }

    for (uint32_t i = 0; i < pages_needed; i++) set_bit(start_page + i, 1);

    // Simple bump allocator for the next search
    mm.next_free = start_page + pages_needed;
    return (void*)(start_page * PAGE_SIZE);
}

/* --- Status Functions --- */

uint32_t mm_get_total(void) { return mm.total_memory; }

uint32_t mm_get_used(void) {
    uint32_t used = 0;
    for (uint32_t i = 0; i < mm.total_pages; i++) {
        if (get_bit(i)) used++;
    }
    return used * PAGE_SIZE;
}

uint32_t mm_get_free(void) { return mm_get_total() - mm_get_used(); }

/**
 * mm_dump_info - Visual memory report for BlueOS
 */
void mm_dump_info(void) {
    printk(WHITE, "\n[ MEMORY REPORT ]\n");
    printk(GRAY, " Total: %d MB\n", mm_get_total() / (1024*1024));
    printk(GRAY, " Free:  %d MB\n", mm_get_free() / (1024*1024));
    printk(GRAY, " Used:  %d MB\n", mm_get_used() / (1024*1024));
    printk(WHITE, "-----------------\n");
}