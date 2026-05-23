/*
 * BlueOS mm/memory.c - Universal Physical Page Frame Allocator
 */

#define MEMORY_C
#include <mm/memory.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <multiboot.h>

#define PAGE_SIZE         4096
struct memory_manager mm = {0};
static uint8_t bitmap_storage[BITMAP_SIZE];

extern uint8_t _kernel_start;
extern uint8_t _kernel_end;


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


void mm_init_universal(uintptr_t ram_start, uint64_t ram_size) {
    if (mm.initialized) return;

    mm.total_memory = ram_size;
    mm.total_pages = ram_size / PAGE_SIZE;
    mm.bitmap = bitmap_storage;
    mm.bitmap_size = (mm.total_pages + 7) / 8;

    // Clear bitmap (all pages free)
    mm_memset(mm.bitmap, 0, BITMAP_SIZE);

    // Calculate kernel boundaries
    uintptr_t k_start = (uintptr_t)&_kernel_start;
    uintptr_t k_end   = (uintptr_t)&_kernel_end;

    // Mark kernel pages as used in the bitmap
    uint32_t start_page = (k_start - ram_start) / PAGE_SIZE;
    uint32_t end_page   = (k_end - ram_start + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = start_page; i < end_page; i++) {
        set_bit(i, 1);
    }

    mm.next_free = end_page;
    mm.initialized = true;
}


void* kmalloc(uint32_t size) {
    if (!mm.initialized || size == 0) return 0;

    uint32_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t consecutive = 0;
    uint32_t start_page = 0;

    // First-Fit
    for (uint32_t i = mm.next_free; i < mm.total_pages; i++) {
        if (!get_bit(i)) {
            if (++consecutive == pages_needed) {
                start_page = i - pages_needed + 1;
                goto found;
            }
        } else {
            consecutive = 0;
        }
    }
    return 0; // OOM

found:
    for (uint32_t i = 0; i < pages_needed; i++) set_bit(start_page + i, 1);
    mm.next_free = start_page + pages_needed;

    return (void*)(uintptr_t)(start_page * PAGE_SIZE);
}


void* mm_memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

void* mm_memcpy(void* dest, const void* src, size_t n) {
    char* d = dest;
    const char* s = src;
    while (n--) {
        *d++ = *s++;
    }
    return d;
}

#if defined(x86)
uint32_t total_memory_mb = 0;

void x86_memory_prepare() {
    total_memory_mb = 128;
}

void mm_init(void* arch_data) {
    struct multiboot_info* mbi = (struct multiboot_info*)arch_data;

    // Check if the bootloader provided memory information
    if (!(mbi->flags & MULTIBOOT_INFO_MEMORY)) {
        return; // This will trigger the panic in k_main
    }

    uint64_t high_mem_bytes = (uint64_t)mbi->mem_upper * 1024;

    mm_init_universal(0x100000, high_mem_bytes);
}
uint32_t mm_get_total(void) {
    if (!mm.initialized) return 0;
    return (uint32_t)mm.total_memory;
}
#endif
