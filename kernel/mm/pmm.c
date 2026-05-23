#include <stdint.h>
#include <mm/memory.h>
#include <stddef.h>
#include <kernel/printk.h>

#define PAGE_SIZE 4096
#define MAX_PAGES 1024
static uint8_t page_bitmap[MAX_PAGES];
extern uintptr_t _end;
extern struct memory_manager mm;
void* kalloc_page() {
    for (int i = 0; i < MAX_PAGES; i++) {
        if (page_bitmap[i] == 0) {
            page_bitmap[i] = 1;


            uintptr_t phys_addr = (uintptr_t)&_end + (i * PAGE_SIZE);

            for (int b = 0; b < PAGE_SIZE; b++) {
                ((char*)phys_addr)[b] = 0;
            }

            return (void*)phys_addr;
        }
    }
    return (void*)0; // Out of memory!
}

void kfree_page(void* ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t base = (uintptr_t)&_end;

    int index = (addr - base) / PAGE_SIZE;
    if (index >= 0 && index < MAX_PAGES) {
        page_bitmap[index] = 0;
    }
}

void* pmm_alloc_frame() {
    if (!mm.initialized) return NULL;

    /* 1. Search for a free frame in the bitmap */
    /* We start searching from mm.next_free to optimize speed */
    for (uint32_t i = mm.next_free; i < mm.total_pages; i++) {
        uint32_t byte = i / 8;
        uint32_t bit  = i % 8;

        /* Check if the bit is 0 (Free) */
        if (!(mm.bitmap[byte] & (1 << bit))) {

            /* 2. Mark as occupied (Set bit to 1) */
            mm.bitmap[byte] |= (1 << bit);

            /* 3. Update next_free for the next allocation */
            mm.next_free = i + 1;

            /* 4. Calculate the physical address */
            /* Address = Page Index * Page Size (4096) */
            uintptr_t paddr = (uintptr_t)i * PAGE_SIZE;

            /* Optional: Log the allocation for debugging */
            // pr_info("PMM: Allocated frame at 0x%p\n", paddr);

            return (void*)paddr;
        }
    }

    /* 5. If no frame was found from next_free, wrap around and search from the start */
    /* (This handles fragmentation) */
    for (uint32_t i = 0; i < mm.next_free; i++) {
        uint32_t byte = i / 8;
        uint32_t bit  = i % 8;

        if (!(mm.bitmap[byte] & (1 << bit))) {
            mm.bitmap[byte] |= (1 << bit);
            mm.next_free = i + 1;
            return (void*)(uintptr_t)(i * PAGE_SIZE);
        }
    }

    /* 6. Out of Physical Memory (OOM) */
    pr_err("PMM: System is out of physical memory!\n");
    return NULL;
}

void pmm_free_frame(void* paddr) {
    uint32_t frame_index = (uintptr_t)paddr / PAGE_SIZE;
    uint32_t byte = frame_index / 8;
    uint32_t bit  = frame_index % 8;

    /* Mark as free (Set bit to 0) */
    mm.bitmap[byte] &= ~(1 << bit);

    /* Optimization: Update next_free if the freed frame is earlier */
    if (frame_index < mm.next_free) {
        mm.next_free = frame_index;
    }
}
