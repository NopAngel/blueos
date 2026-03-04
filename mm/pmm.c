#include <blueos/types.h>
#include <blueos/printk.h>

#define PAGE_SIZE 4096
#define BITMAP_SIZE (1024 * 1024 / 8) 

uint32_t* bitmap;
uint32_t total_blocks;
uint32_t used_blocks;

void pmm_set_bit(uint32_t bit) {
    bitmap[bit / 32] |= (1 << (bit % 32));
}

void pmm_unset_bit(uint32_t bit) {
    bitmap[bit / 32] &= ~(1 << (bit % 32));
}

int pmm_test_bit(uint32_t bit) {
    return bitmap[bit / 32] & (1 << (bit % 32));
}

void pmm_init(uint32_t mem_size, uint32_t bitmap_addr) {
    bitmap = (uint32_t*)bitmap_addr;
    total_blocks = mem_size / PAGE_SIZE;
    used_blocks = total_blocks;

    for (uint32_t i = 0; i < total_blocks / 32; i++) {
        bitmap[i] = 0xFFFFFFFF;
    }
}

void pmm_init_region(uint32_t base, uint32_t size) {
    uint32_t align = base / PAGE_SIZE;
    uint32_t blocks = size / PAGE_SIZE;

    for (; blocks > 0; blocks--) {
        pmm_unset_bit(align++);
        used_blocks--;
    }
}

int pmm_find_first_free() {
    for (uint32_t i = 0; i < total_blocks / 32; i++) {
        if (bitmap[i] != 0xFFFFFFFF) {
            for (int j = 0; j < 32; j++) {
                if (!(bitmap[i] & (1 << j))) {
                    return i * 32 + j;
                }
            }
        }
    }
    return -1;
}

void* pmm_alloc_block() {
    int free_block = pmm_find_first_free();
    if (free_block == -1) return NULL; // Out of Memory!

    pmm_set_bit(free_block);
    used_blocks++;
    return (void*)(free_block * PAGE_SIZE);
}

void pmm_free_block(void* addr) {
    uint32_t block = (uint32_t)addr / PAGE_SIZE;
    pmm_unset_bit(block);
    used_blocks--;
}