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
static uint8_t protection_bitmap[BITMAP_SIZE];

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

static void set_protected(uint32_t bit, int value) {
    uint32_t byte = bit / 8;
    uint32_t bit_in_byte = bit % 8;
    if (value) mm.protection_bitmap[byte] |= (1 << bit_in_byte);
    else mm.protection_bitmap[byte] &= ~(1 << bit_in_byte);
}

static int get_protected(uint32_t bit) {
    uint32_t byte = bit / 8;
    uint32_t bit_in_byte = bit % 8;
    return (mm.protection_bitmap[byte] >> bit_in_byte) & 1;
}

static int page_available(uint32_t bit) {
    return !get_bit(bit) && !get_protected(bit);
}

void mm_protect_region(uintptr_t base, uint32_t size) {
    if (!mm.initialized || size == 0) return;

    if (base < mm.phys_limit_start) {
        base = mm.phys_limit_start;
    }

    uintptr_t start_bit = (base - mm.phys_limit_start) / PAGE_SIZE;
    uintptr_t end_bit = ((base + size + PAGE_SIZE - 1) - mm.phys_limit_start) / PAGE_SIZE;

    for (uint32_t i = (uint32_t)start_bit; i < (uint32_t)end_bit; i++) {
        set_bit(i, 1);
        set_protected(i, 1);
    }
}

bool mm_is_page_protected(uint32_t frame_index) {
    return get_protected(frame_index) != 0;
}

bool mm_is_kaslr_enabled(void) {
    return mm.kaslr_enabled;
}

uintptr_t mm_get_kernel_slide(void) {
    return mm.kaslr_slide;
}

void mm_init_universal(uintptr_t ram_start, uint64_t ram_size) {
    if (mm.initialized) return;

    mm.total_memory = ram_size;
    mm.total_pages = ram_size / PAGE_SIZE;
    mm.bitmap = bitmap_storage;
    mm.protection_bitmap = protection_bitmap;
    mm.bitmap_size = (mm.total_pages + 7) / 8;

    // Clear bitmap (all pages free)
    mm_memset(mm.bitmap, 0, BITMAP_SIZE);
    mm_memset(mm.protection_bitmap, 0, BITMAP_SIZE);

    mm.phys_limit_start = ram_start;

    // Detect whether the kernel was relocated via KASLR
    uintptr_t k_start = (uintptr_t)&_kernel_start;
    uintptr_t k_end   = (uintptr_t)&_kernel_end;
    mm.kaslr_slide = k_start - KERNEL_START;
    mm.kaslr_enabled = (mm.kaslr_slide != 0);

    if (mm.kaslr_enabled) {
        printk("[KASLR] Detected kernel slide: 0x%08x\n", (uint32_t)mm.kaslr_slide);
    } else {
        printk("[KASLR] Kernel loaded at expected base 0x%08x\n", (uint32_t)k_start);
    }

    // Mark kernel pages as used and protected in the bitmap
    uint32_t start_page = (k_start - ram_start) / PAGE_SIZE;
    uint32_t end_page   = (k_end - ram_start + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = start_page; i < end_page; i++) {
        set_bit(i, 1);
        set_protected(i, 1);
    }

    mm.initialized = true;
    // Protect one guard page immediately after the kernel to catch overflows
    mm_protect_region(k_end, PAGE_SIZE);
}


void* kmalloc(uint32_t size) {
    if (!mm.initialized || size == 0) return 0;

    uint32_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t consecutive = 0;
    uint32_t start_page = 0;

    // First-Fit
    for (uint32_t i = mm.next_free; i < mm.total_pages; i++) {
        if (page_available(i)) {
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
    for (uint32_t i = 0; i < pages_needed; i++) {
        set_bit(start_page + i, 1);
    }
    mm.next_free = start_page + pages_needed;

    return (void*)(uintptr_t)((start_page * PAGE_SIZE) + mm.phys_limit_start);
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


uint64_t mm_get_free_memory(void) {
    if (!mm.initialized) return 0;

    uint32_t free_pages = 0;
    for (uint32_t i = 0; i < mm.total_pages; i++) {
        if (!get_bit(i)) {
            free_pages++;
        }
    }
    return (uint64_t)free_pages * PAGE_SIZE;
}
